#ifndef GFX_H
#define GFX_H

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>

/*  =====================================================================================

        NOTE(acol): 4 Bytes: Id of the resource we call the method on
                    2 Bytes: Opcode of the method being called
                    2 Bytes: Size of the message message
                    x Bytes: Arguments that are passed in the method

    ===================================================================================== */

#define WAYLAND_HEADER_SIZE 8u

#define WAYLAND_DISPLAY_OBJECT_ID 1u
#define WAYLAND_WL_REGISTRY_EVENT_GLOBAL (u16)0
#define WAYLAND_SHM_POOL_EVENT_FORMAT (u16)0
#define WAYLAND_WL_BUFFER_EVENT_RELEASE (u16)0
#define WAYLAND_XDG_WM_BASE_EVENT_PING (u16)0
#define WAYLAND_XDG_TOPLEVEL_EVENT_CONFIGURE (u16)0
#define WAYLAND_XDG_TOPLEVEL_EVENT_CLOSE (u16)1
#define WAYLAND_XDG_SURFACE_EVENT_CONFIGURE (u16)0
#define WAYLAND_WL_DISPLAY_GET_REGISTRY_OPCODE (u16)1
#define WAYLAND_WL_REGISTRY_BIND_OPCODE (u16)0
#define WAYLAND_WL_COMPOSITOR_CREATE_SURFACE_OPCODE (u16)0
#define WAYLAND_XDG_WM_BASE_PONG_OPCODE (u16)3
#define WAYLAND_XDG_SURFACE_ACK_CONFIGURE_OPCODE (u16)4
#define WAYLAND_WL_SHM_CREATE_POOL_OPCODE (u16)0
#define WAYLAND_XDG_WM_BASE_GET_XDG_SURFACE_OPCODE (u16)2
#define WAYLAND_WL_SHM_POOL_CREATE_BUFFER_OPCODE (u16)0
#define WAYLAND_WL_SURFACE_ATTACH_OPCODE (u16)1
#define WAYLAND_XDG_SURFACE_GET_TOPLEVEL_OPCODE (u16)1
#define WAYLAND_WL_SURFACE_COMMIT_OPCODE (u16)6
#define WAYLAND_WL_DISPLAY_ERROR_EVENT (u16)0
#define WAYLAND_FORMAT_XRGB8888 1u

#define COLOR_CHANNELS 4u

typedef struct wayland_context {
    u32 CurrentId;
    i32 SocketFd;

    u32 RegistryId;

    u32 Width;
    u32 Height;

    ring_buffer RingBuffer;

    // TODO
    u32 wl_shm;
    u32 wl_shm_pool;
    u32 wl_buffer;
    u32 xdg_wm_base;
    u32 xdg_surface;
    u32 wl_compositor;
    u32 wl_surface;
    u32 xdg_toplevel;
    u32 stride;
    u32 shm_pool_size;
    i32 shm_fd;
    u8 *shm_pool_data;

    //  state_state_t state;
} wayland_context;

typedef struct wayland_header {
    u32 ResourceId;
    u16 Opcode;
    u16 Size;
} wayland_header;

static void WaylandDisplayConnect(wayland_context *Context);
static void WaylandGetRegistry(wayland_context *Context);

#define WaylandMessagePush(Buff, Pos, Value, Type) \
    Statement(*(Type *)((u8 *)(Buff) + (Pos)) = (Type)(Value); (Pos) += sizeof(Type);)

#endif  // GFX_H
