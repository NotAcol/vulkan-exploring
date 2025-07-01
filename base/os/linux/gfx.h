#ifndef GFX_H
#define GFX_H

#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

#define VK_USE_PLATFORM_WAYLAND_KHR

#if WAYLAND_DEBUG
    #define WaylandLog(...) printf(__VA_ARGS__)
#else
    #define WaylandLog(...)
#endif

#define VULKAN_WINDOWING_DEVICE_EXTENSIONS                                                   \
    "VK_KHR_external_memory", "VK_KHR_external_memory_fd", "VK_EXT_external_memory_dma_buf", \
        "VK_EXT_image_drm_format_modifier", "VK_KHR_external_semaphore",

#define VULKAN_WINDOWING_INSTANCE_EXTENSIONS "VK_KHR_external_memory_capabilities",
//"VK_KHR_surface", "VK_KHR_wayland_surface",

#define WL_DISPLAY_ID 1
#define WL_DISPLAY_EVENT_ERROR 0
#define WL_REGISTRY_EVENT_GLOBAL 0

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

} wayland_context;

typedef struct wayland_header {
    u32 ObjectId;
    u16 OpCode;
    u16 Size;
} wayland_header;

#define REQUIRED_REGISTRY_ITEM \
    X(xdg_wm_base) \
    X(wl_compositor) \


//typedef enum needed_registry_item {
//        
//}

static void *WaylandGetMessages(i32 Fd, ring_buffer *RingBuffer);
static void WaylandDisplayGetRegistry(i32 Fd, u32 CurrentId);
static void WaylandRegistryBind(i32 Fd, u32 RegistryId, u32 Name, string8 Interface, u32 Version, u32 Id);
static void WaylandInit(wayland_context *WlCtx, u32 Height, u32 Width, string8 Title);

#endif  // GFX_H
