#ifndef GFX_H
#define GFX_H

// #include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

#define VK_USE_PLATFORM_WAYLAND_KHR

#if WAYLAND_DEBUG
    #define WaylandLog(...) printf(__VA_ARGS__)
#else
    #define WaylandLog(...)
#endif

#define VULKAN_WINDOWING_EXTENSIONS (char **){"VK_KHR_surface", "VK_KHR_wayland_surface"}

#define DISPLAY_OBJECT_ID 1

#define WL_DISPLAY_GET_REGISTRY_OPCODE 1
#define WL_DISPLAY_ERROR_EVENT 0
#define WL_REGISTRY_BIND_OPCODE 0
#define WL_REGISTRY_EVENT_GLOBAL 0
#define WL_SURFACE_COMMIT_OPCODE 6
#define WL_SURFACE_ATTACH_OPCODE 1
#define WL_SURFACE_DESTROY_OPCODE 0
#define XDG_WM_BASE_DESTROY_OPCODE 0
#define XDG_WM_BASE_EVENT_PING 0
#define XDG_WM_BASE_PONG_OPCODE 3
#define XDG_WM_BASE_GET_XDG_SURFACE_OPCODE 2
#define XDG_SURFACE_EVENT_CONFIGURE 0
#define XDG_SURFACE_ACK_CONFIGURE_OPCODE 4
#define XDG_SURFACE_GET_TOPLEVEL_OPCODE 1
#define XDG_TOPLEVEL_EVENT_CONFIGURE 0
#define XDG_TOPLEVEL_EVENT_CLOSE 1
#define XDG_TOPLEVEL_EVENT_CONFIGURE_BOUNDS 2
#define XDG_TOPLEVEL_EVENT_WM_CAPABILITIES 3
#define XDG_TOPLEVEL_SET_TITLE_OPCODE 2
#define XDG_TOPLEVEL_DESTROY_OPCODE 0
#define WL_COMPOSITOR_CREATE_SURFACE_OPCODE 0
#define XDG_SURFACE_DESTROY_OPCODE 0

#define TOPLEVEL_STATES  \
    X(MAXIMIZED)         \
    X(FULLSCREEN)        \
    X(RESIZING)          \
    X(ACTIVATED)         \
    X(TILED_LEFT)        \
    X(TILED_RIGHT)       \
    X(TILED_TOP)         \
    X(TILED_BOTTOM)      \
    X(SUSPENDED)         \
    X(CONSTRAINED_LEFT)  \
    X(CONSTRAINED_RIGHT) \
    X(CONSTRAINED_TOP)   \
    X(CONSTRAINED_BOTTOM)

#define TOPLEVEL_WM_CAPABILITIES \
    X(WINDOW_MENU)               \
    X(MAXIMIZE)                  \
    X(FULLSCREEN)                \
    X(MINIMIZE)

typedef enum toplevel_states {
#define X(S) S,
    TOPLEVEL_STATES
#undef X
} toplevel_states;

typedef enum wayland_state {
    WLSTATE_SurfaceCommited,
    WLSTATE_NewSize,
    WLSTATE_ReadyToResize,
    WLSTATE_Error,
    WLSTATE_Quit
} wayland_state;

typedef struct wayland_context {
    u32 CurrentId;
    i32 Fd;

    u32 Registry;
    u32 WlCompositor;
    u32 WlSurface;

    u32 XdgWmBase;
    u32 XdgSurface;
    u32 XdgToplevel;

    u32 Width;
    u32 Height;

    ring_buffer RingBuffer;

    wayland_state State;

} wayland_context;

typedef struct wayland_header {
    u32 ObjectId;
    u16 OpCode;
    u16 Size;
} wayland_header;

static void WaylandInit(wayland_context *WlCtx, u32 Height, u32 Width, string8 Title);
static u32 WaylandRegistryBind(u32 Fd, u32 CurrentId, u32 Registry, u32 Name, u32 Version, u32 InterfaceLen,
                               char *Interface);
static void WaylandCommitSurface(wayland_context *WlCtx);
static void WaylandPollEvents(wayland_context *WlCtx);
static b32 WaylandShouldResize(wayland_context *WlCtx);
// static void WaylandWindowClose(wayland_context *WlCtx);
static void WaylandTerminate(wayland_context *WlCtx);

#endif  // GFX_H
