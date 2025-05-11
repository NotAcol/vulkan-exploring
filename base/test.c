#include "base_include.h"
thread_local tctx* TctxThreadLocal;
#include "base_include.c"

int main(void) {
    arena* Arena = ArenaAlloc();

    shishua_state Seed = ShishuaSeed((u64[4]){0, 0, 0, 0});

    u64* RngNums = PushArrayNoZero(Arena, u64, 400);
    // u64* RngNums = ArenaPushNoZero(Arena, 8 * 400);

    ShishuaGen(&Seed, RngNums, 400);
    printf("[\n");
    for (i32 i = 0; i < 100; i++) {
        printf("%22llu %22llu %22llu %22llu\n", RngNums[i], RngNums[i + 1], RngNums[i + 2], RngNums[i + 3]);
    }
    printf("]\n");
    return 0;
}
