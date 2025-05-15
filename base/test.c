#include "base_include.h"

thread_local tctx* TctxThreadLocal;
#include "base_include.c"

int main(void) {
    tctx Tctx;
    TctxInitAndEquip(&Tctx);
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

    struct erm {
        string8 Str;
        u64 Num;
    };
    struct erm B = {0};

    printf("%llu\n", RngNums[B.Num++]);
    printf("%llu", B.Num);

#define RingBufferLength 64

    ring_buffer RingBuffer = RingBufferAlloc(RingBufferLength * 4);

    u64 ReadBuffer[RingBufferLength] = {0};

    RingBufferWrite(RingBuffer, RngNums, sizeof(*RngNums) * RingBufferLength);
    RingBufferRead(RingBuffer, ReadBuffer, sizeof(*ReadBuffer) * RingBufferLength);

    wayland_context Context = {0};

    WaylandDisplayConnect(&Context);

    wayland_message HeaderTest = WaylandGetRegistry(&Context);

    RingBufferWrite(RingBuffer, &HeaderTest.Header, sizeof(HeaderTest.Header));

    RingBufferRelease(RingBuffer);
    TctxRelease();
    return 0;
}
