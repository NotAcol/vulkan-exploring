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
    ARENA_NoChainGrow = (1 << 0),
    ARENA_LargePages = (1 << 1),
    ARENA_FreeList = (1 << 2)
} arena_flags;

// NOTE(acol): for padding to cache line
#define ARENA_STRUCT_SIZE 128
#define ARENA_DEFAULT_RESERVE_SIZE MB(64)
#define ARENA_DEFAULT_COMMIT_SIZE KB(64)
#define ARENA_DEFAULT_FLAGS 0

typedef struct arena {
    struct arena *Current = 0;
    struct arena *Prev = 0;
    u64 Reserved = 0;
    u64 Commited = 0;
    u64 BasePos = 0;
    u64 Pos = 0;
    u64 Flags = 0;
    arena *FreeLast = 0;
    u64 FreeSize = 0;
} arena;
StaticAssert(sizeof(arena) <= ARENA_STRUCT_SIZE, arena struct too long);

typedef struct temp_arena {
    arena *Arena = 0;
    u64 OldPos = 0;
} temp_arena;

typedef struct arena_alloc_params {
    // NOTE(acol): Default is chain growing
    u64 Flags = ARENA_DEFAULT_FLAGS;
    u64 ReserveSize = ARENA_DEFAULT_RESERVE_SIZE;
    u64 CommitSize = ARENA_DEFAULT_COMMIT_SIZE;
    void *BackingBuffer = 0;
} arena_alloc_params;

static arena *ArenaAlloc(arena_alloc_params Params);
static void ArenaRelease(arena *Arena);
static u64 ArenaPos(arena *Arena);
static void *ArenaPush(arena *Arena, u64 Size, u64 Align = 1);
static void *ArenaPushZero(arena *Arena, u64 Size, u64 Align = 1);
static void ArenaPopTo(arena *Arena, u64 Pos);
static void ArenaReset(arena *Arena);
static void ArenaPop(arena *Arena, u64 Size);
static temp_arena TempBegin(arena *Arena);
static void TempEnd(temp_arena Temp);

#endif  // ARENA_H
