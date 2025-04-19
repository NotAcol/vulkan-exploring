#include "base_include.h"
#include "base_include.cpp"

int main(void) {
    BeginProfile();
    v4 blah = {1.0, 2.0, 3.0, 4.0};
    v4 blah2 = {1.0, 2.0, 3.0, 4.0};
    blah = blah + blah2;
    {
        ProfileBlock("prints");
        printf("%f %f %f %f\n", (f64)blah.X, (f64)blah.Y, (f64)blah.Z, (f64)blah.W);

        printf("%lu\n", sizeof(arena));
    }

    arena_alloc_params ArenaParams = {
        .Flags = ARENA_FreeList, .ReserveSize = OS_PAGE_SIZE, .CommitSize = OS_PAGE_SIZE, .BackingBuffer = 0};

    arena* Arena = ArenaAlloc(ArenaParams);

    for (u32 i = 0; i < 4; i++) {
        u8* Base = (u8*)ArenaPush(Arena, KB(4));
        for (u32 j = 0; j < KB(4); j++) {
            Base[j] = 'A';
        }
    }
    for (u32 i = 0; i < 2; i++) {
        ArenaPop(Arena, KB(4));
    }
    u8* Base = (u8*)ArenaPush(Arena, KB(4));
    for (u32 i = 0; i < KB(4); i++) {
        Base[i] = 'A';
    }

    EndAndPrintProfile();
    return 0;
}

PROFILER_END_OF_COMPILATION_UNIT;
