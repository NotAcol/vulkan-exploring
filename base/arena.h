#ifndef ARENA_H
#define ARENA_H

#include "intrinsics.h"

/*
 NOTE(acol):    Arena free list isn't per item, instead of releasing a link when using
                chain growth its put into a single linked list and reused when needed.
                If you want per item a simple pool allocator can work like this:

struct entity {
    entity *Next;
    v2 Data;
};

struct entity_pool {
    arena *Arena;
    entity *FirstFreeEntity;
};

static entity *EntityAlloc(entity_pool Pool) {
    entity *FreeSlot = Pool->FirstFreeEntity;
    if (FreeSlot) {
        Pool->FirstFreeEntity = FreeSlot->Next;
        MemoryZeroStruct(FreeSlot);
    } else {
        FreeSlot = ArenaPush(Pool.Arena, sizeof(entity));
        MemoryZeroStruct(FreeSlot);
    }
    return FreeSlot;
}

static void EntityRelease(entity_pool Pool, entity Entity) {
    Entity.Next = Pool.FirstFreeEntity;
    Pool.FirstFreeEntity = Entity;
}
*/

typedef enum arena_flags {
    Arena_NoChainGrow = (1 << 0),
    Arena_LargePages = (1 << 1),
    Arena_FreeList = (1 << 2)
} arena_flags;

// NOTE(acol): for padding to cache line
#define ARENA_STRUCT_SIZE 128
#define ARENA_DEFAULT_RESERVE_SIZE MB(64)
#define ARENA_DEFAULT_COMMIT_SIZE KB(64)
#define ARENA_DEFAULT_FLAGS 0

typedef struct arena {
    struct arena *Current;
    struct arena *Prev;
    u64 Reserved;
    u64 Commited;
    u64 BasePos;
    u64 Pos;
    u64 Flags;
    struct arena *FreeLast;
    u64 FreeSize;
} arena;
StaticAssert(sizeof(arena) <= ARENA_STRUCT_SIZE, arena struct too long);

typedef struct temp_arena {
    arena *Arena;
    u64 OldPos;
} temp_arena;

typedef struct arena_alloc_params {
    // NOTE(acol): Default is chain growing
    u64 Flags;
    u64 ReserveSize;
    u64 CommitSize;
    void *BackingBuffer;
} arena_alloc_params;

static arena *ArenaAlloc_(arena_alloc_params Params);
static void ArenaRelease(arena *Arena);

static u64 ArenaPos(arena *Arena);
static void *ArenaPushNoZeroAligned(arena *Arena, u64 Size, u64 Align);
static void *ArenaPushAligned(arena *Arena, u64 Size, u64 Align);

static void ArenaPopTo(arena *Arena, u64 Pos);
static void ArenaPop(arena *Arena, u64 Size);
static void ArenaReset(arena *Arena);

static temp_arena TempBegin(arena *Arena);
static void TempEnd(temp_arena Temp);

#define ArenaAlloc(...)                                                         \
    ArenaAlloc_((arena_alloc_params){.Flags = ARENA_DEFAULT_FLAGS,              \
                                     .ReserveSize = ARENA_DEFAULT_RESERVE_SIZE, \
                                     .CommitSize = ARENA_DEFAULT_COMMIT_SIZE,   \
                                     .BackingBuffer = 0,                        \
                                     __VA_ARGS__})

#define ArenaPushNoZero(Arena, Size) ArenaPushNoZeroAligned((Arena), (Size), 4)
#define ArenaPush(Arena, Size) ArenaPushAligned((Arena), (Size), 4)

#define PushArrayNoZeroAligned(Arena, Type, Count, Align) \
    (Type *)ArenaPushNoZeroAligned((Arena), sizeof(Type) * (Count), (Align))
#define PushArrayAligned(Arena, Type, Count, Align) \
    (Type *)ArenaPushAligned((Arena), sizeof(Type) * (Count), (Align))

#define PushArrayNoZero(Arena, Type, Count) \
    (Type *)ArenaPushNoZeroAligned((Arena), sizeof(Type) * (Count), AlignOf(Type))
#define PushArray(Arena, Type, Count) (Type *)ArenaPushAligned((Arena), sizeof(Type) * (Count), AlignOf(Type))

#endif  // ARENA_H
