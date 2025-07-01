#include "base/base_include.h"
thread_local tctx* TctxThreadLocal;
#include "base/base_include.c"

#include <stdio.h>
int main(void) {
    tctx Tctx;
    TctxInitAndEquip(&Tctx);

    wayland_context WlCtx = {0};
    WaylandInit(&WlCtx, 100, 100, String8Lit("erm"));

    return 0;
}
