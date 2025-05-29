#include "base/base_include.h"

thread_local tctx *TctxThreadLocal;
#include "base/base_include.c"

int main(void) {
    tctx Tctx;
    TctxInitAndEquip(&Tctx);

    wayland_context WlCtx = {0};

    WaylandInit(&WlCtx, 100, 100, String8Lit("Test"));
    WaylandPollEvents(&WlCtx);

    WaylandTerminate(&WlCtx);

    TctxRelease();
    return 0;
}

// internal OS_Handle      os_window_open(Rng2F32 rect, OS_WindowFlags flags, String8 title);
// internal void           os_window_close(OS_Handle window);
// internal void           os_window_set_title(OS_Handle window, String8 title);
// internal void           os_window_first_paint(OS_Handle window);
// internal void           os_window_focus(OS_Handle window);
// internal B32            os_window_is_focused(OS_Handle window);
// internal B32            os_window_is_fullscreen(OS_Handle window);
// internal void           os_window_set_fullscreen(OS_Handle window, B32 fullscreen);
// internal B32            os_window_is_maximized(OS_Handle window);
// internal void           os_window_set_maximized(OS_Handle window, B32 maximized);
// internal B32            os_window_is_minimized(OS_Handle window);
// internal void           os_window_set_minimized(OS_Handle window, B32 minimized);
// internal void           os_window_bring_to_front(OS_Handle window);
// internal void           os_window_set_monitor(OS_Handle window, OS_Handle monitor);
// internal void           os_window_clear_custom_border_data(OS_Handle handle);
// internal void           os_window_push_custom_title_bar(OS_Handle handle, F32 thickness);
// internal void           os_window_push_custom_edges(OS_Handle handle, F32 thickness);
// internal void           os_window_push_custom_title_bar_client_area(OS_Handle handle, Rng2F32 rect);
// internal Rng2F32        os_rect_from_window(OS_Handle window);
// internal Rng2F32        os_client_rect_from_window(OS_Handle window);
// internal F32            os_dpi_from_window(OS_Handle window);
