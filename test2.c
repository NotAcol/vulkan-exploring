#include "base/base_include.h"

thread_local tctx *TctxThreadLocal;
#include "base/base_include.c"

int main(void) {
    tctx Tctx;
    TctxInitAndEquip(&Tctx);

    wayland_context WlCtx = {0};

    WaylandInit(&WlCtx, 100, 100, String8Lit("Test"));
    WaylandPollEvents(&WlCtx);

    TctxRelease();
    return 0;
}
