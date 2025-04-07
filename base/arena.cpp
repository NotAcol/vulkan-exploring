#include "intrinsics.h"

static arena *ArenaAlloc(arena_alloc_params Params) {
    u64 ReserveSize = ClampBot(Params.ReserveSize, ARENA_STRUCT_SIZE);
    u64 CommitSize = ClampBot(Params.CommitSize, ARENA_STRUCT_SIZE);

    if (Params.Flags & ARENA_LargePages) {
        ReserveSize = AlignPow2(ReserveSize, OS_LARGE_PAGE_SIZE);
        CommitSize = AlignPow2(CommitSize, OS_LARGE_PAGE_SIZE);
    } else {
        ReserveSize = AlignPow2(ReserveSize, OS_PAGE_SIZE);
        CommitSize = AlignPow2(CommitSize, OS_PAGE_SIZE);
    }

    CommitSize = ClampTop(CommitSize, ReserveSize);

    void *Base = 0;
    if (Params.BackingBuffer) {
        Assert(ReserveSize == Params.ReserveSize || CommitSize == Params.CommitSize);
        Base = Params.BackingBuffer;
    } else {
        if (Params.Flags & ARENA_LargePages) {
            Base = OsReserveLarge(ReserveSize);
            OsCommitLarge(Base, CommitSize);
        } else {
            Base = OsReserve(ReserveSize);
            OsCommit(Base, CommitSize);
        }
    }

    // Assert(Base != 0);
    if (!Base) return (arena *)0;

    arena *Arena = (arena *)Base;
    Arena->Current = Arena;
    Arena->Prev = 0;
    Arena->BasePos = 0;
    Arena->Reserved = ReserveSize;
    Arena->Commited = CommitSize;
    Arena->Pos = ARENA_STRUCT_SIZE;
    Arena->Flags = Params.Flags;
    Arena->FreeLast = 0;
    Arena->FreeSize = 0;

#if defined(ARENA_LOGGING)
    dprintf(2, "Arena Allocated\n");
    dprintf(2, "  Reserved: %llu\n", Arena->Reserved);
    dprintf(2, "  Commited: %llu\n", Arena->Commited);
    dprintf(2, "  BasePos: %llu\n", Arena->BasePos);
    dprintf(2, "  Pos: %llu\n", Arena->Pos);
#endif

    AsanPoison((u8 *)Arena + ARENA_STRUCT_SIZE, CommitSize - ARENA_STRUCT_SIZE);

    return Arena;
}

static void ArenaRelease(arena *Arena) {
#if defined(ARENA_LOGGING)
    dprintf(2, "Arena Released\n");
    dprintf(2, "  Reserved: %llu\n", Arena->Reserved);
    dprintf(2, "  Commited: %llu\n", Arena->Commited);
    dprintf(2, "  BasePos: %llu\n", Arena->BasePos);
#endif
    if (Arena->Flags & ARENA_FreeList) {
        for (arena *Current = Arena->FreeLast, *Temp = 0; Current != 0; Current = Temp) {
            Temp = Current->Prev;
            OsRelease(Current, Current->Reserved);
        }
    }
    for (arena *Current = Arena->Current, *Temp = 0; Current != 0; Current = Temp) {
        Temp = Current->Prev;
        OsRelease(Current, Current->Reserved);
    }
}

static u64 ArenaPos(arena *Arena) { return Arena->Current->BasePos + Arena->Current->Pos; }

static void *ArenaPushNoZero(arena *Arena, u64 Size, u64 Align) {
    arena *Current = Arena->Current;
    u64 StartPos = AlignPow2(Current->Pos, Align);
    u64 EndPos = StartPos + Size;

    if (Current->Reserved < EndPos && !(Arena->Flags & ARENA_NoChainGrow)) {
        arena *NewBlock = Arena->FreeLast;

        // NOTE(acol):  This is a separate linked list from the growth chain.
        //              It will fail NewBlock!=0 check if there isnt a free list so no need for flag check
        for (arena *PrevBlock = 0; NewBlock != 0; PrevBlock = NewBlock, NewBlock = NewBlock->Prev) {
            if (NewBlock->Reserved >= AlignPow2(Size, Align)) {
                if (PrevBlock) {
                    PrevBlock->Prev = NewBlock->Prev;
                } else {
                    Arena->FreeLast = NewBlock->Prev;
                }
                Arena->FreeSize -= NewBlock->Reserved;
                // AsanUnpoison((u8*)NewBlock + ARENA_STRUCT_SIZE, NewBlock->ReserveSize - ARENA_STRUCT_SIZE);
            }
        }

        if (NewBlock == 0) {
            u64 ReserveSize = Current->Reserved;
            u64 CommitSize = Current->Commited;
            if (Size + ARENA_STRUCT_SIZE > ReserveSize) {
                ReserveSize = AlignPow2(Size + ARENA_STRUCT_SIZE, Align);
                CommitSize = ReserveSize;
            }
            NewBlock =
                ArenaAlloc({.Flags = Current->Flags, .ReserveSize = ReserveSize, .CommitSize = CommitSize});
        }
        NewBlock->BasePos = Current->BasePos + Current->Reserved;
        SllStackPush_N(Arena->Current, NewBlock, Prev);

        Current = NewBlock;
        StartPos = AlignPow2(Current->Pos, Align);
        EndPos = StartPos + Size;
    }

    if (Current->Commited < EndPos) {
        if (Current->Flags & ARENA_LargePages) {
            u64 CommitSize = AlignPow2(EndPos - Current->Pos, OS_LARGE_PAGE_SIZE);
            OsCommitLarge((u8 *)Current + Current->Commited, CommitSize);
            Current->Commited += CommitSize;
#if defined(ARENA_LOGGING)
            dprintf(2, "Arena Commited on push: %llu\n", CommitSize);
#endif
        } else {
            u64 CommitSize = AlignPow2(EndPos - Current->Pos, OS_PAGE_SIZE);
            OsCommit((u8 *)Current + Current->Commited, CommitSize);
            Current->Commited += CommitSize;
#if defined(ARENA_LOGGING)
            dprintf(2, "Arena Commited on push: %llu\n", CommitSize);
#endif
        }
        dprintf(2, "Arena Current: %p\n", (void *)((u8 *)Current));
        dprintf(2, "Arena Current+StartPos: %p\n", (void *)((u8 *)Current + StartPos));
    }

    void *Ret = 0;
    if (Current->Commited >= EndPos) {
        Ret = (u8 *)Current + StartPos;
        Current->Pos = EndPos;
        AsanUnpoison(Ret, Size);
    }
    Assert(Ret != 0);
    return Ret;
}

static void *ArenaPush(arena *Arena, u64 Size, u64 Align) {
#if defined(ARENA_LOGGING)
    dprintf(2, "Arena Pushed: %llu\n", Size);
#endif
    void *Ret = ArenaPushNoZero(Arena, Size, Align);
    MemoryZero(Ret, Size);
    return Ret;
}

static void ArenaPopTo(arena *Arena, u64 Pos) {
    u64 PopPos = ClampBot(Pos, ARENA_STRUCT_SIZE);
    arena *Current = Arena->Current;
    if (Arena->Flags & ARENA_FreeList) {
        for (arena *Temp = 0; Current->BasePos >= PopPos; Current = Temp) {
            Temp = Current->Prev;
            Current->Pos = ARENA_STRUCT_SIZE;
            Arena->FreeSize += Current->Reserved;
            SllStackPush_N(Arena->FreeLast, Current, Prev);
            AsanPoison((u8 *)Current + ARENA_STRUCT_SIZE, Current->Reserved - ARENA_STRUCT_SIZE);
        }
    } else {
        for (arena *Temp = 0; Current->BasePos >= PopPos; Current = Temp) {
            Temp = Current->Prev;
            OsRelease(Current, Current->Reserved);
        }
    }
    Arena->Current = Current;
    u64 NewPos = PopPos - Current->BasePos;
    // NOTE(acol): just in case of fuckery with wrapping/passing negative numbers;
    Assert(NewPos <= Current->Pos);

    AsanPoison((u8 *)Current + NewPos, Current->Commited - NewPos);
    Current->Pos = NewPos;
}

static void ArenaReset(arena *Arena) { ArenaPopTo(Arena, 0); }

static void ArenaPop(arena *Arena, u64 Size) {
    u64 CurrentPos = ArenaPos(Arena);
    u64 NewPos = CurrentPos - Size;
    // NOTE(acol): overflows negative numbers and whatnot
    if (NewPos < CurrentPos) {
        ArenaPopTo(Arena, NewPos);
    }
}

// TODO(acol): Maybe make these constructor destructor to make it more ergonomic ?

static temp_arena TempBegin(arena *Arena) {
    u64 CurrentPos = ArenaPos(Arena);
    temp_arena Temp = {Arena, CurrentPos};
    return Temp;
}
static void TempEnd(temp_arena Temp) { ArenaPopTo(Temp.Arena, Temp.OldPos); }
