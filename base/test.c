#include "base_include.h"

thread_local tctx *TctxThreadLocal;
#include "base_include.c"

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>

#if WAYLAND_DEBUG
    #define WaylandLog(...) printf(__VA_ARGS__)
#else
    #define WaylandLog(...)
#endif

#define WAYLAND_DISPLAY_OBJECT_ID 1u

#define WAYLAND_WL_DISPLAY_GET_REGISTRY_OPCODE (u16)1

#define WAYLAND_WL_REGISTRY_BIND_OPCODE (u16)0
#define WAYLAND_WL_REGISTRY_EVENT_GLOBAL (u16)0

#define WAYLAND_WL_SURFACE_COMMIT_OPCODE (u16)6
#define WAYLAND_WL_SURFACE_ATTACH_OPCODE (u16)1

#define WAYLAND_XDG_WM_BASE_EVENT_PING (u16)0
#define WAYLAND_XDG_WM_BASE_PONG_OPCODE (u16)3
#define WAYLAND_XDG_WM_BASE_GET_XDG_SURFACE_OPCODE (u16)2

#define WAYLAND_XDG_SURFACE_EVENT_CONFIGURE (u16)0
#define WAYLAND_XDG_SURFACE_ACK_CONFIGURE_OPCODE (u16)4
#define WAYLAND_XDG_SURFACE_GET_TOPLEVEL_OPCODE (u16)1

#define WAYLAND_XDG_TOPLEVEL_EVENT_CONFIGURE (u16)0
#define WAYLAND_XDG_TOPLEVEL_EVENT_CLOSE (u16)1
#define WAYLAND_XDG_TOPLEVEL_SET_TITLE_OPCODE (u16)2
#define WAYLAND_XDG_TOPLEVEL_DESTROY_OPCODE (u16)0

#define WAYLAND_WL_COMPOSITOR_CREATE_SURFACE_OPCODE (u16)0

#define WAYLAND_WL_DISPLAY_ERROR_EVENT (u16)0

#define WAYLAND_FORMAT_XRGB8888 1u

#define WAYLAND_XDG_SURFACE_DESTROY_OPCODE (u16)0

#define WAYLAND_WL_SURFACE_DESTROY_OPCODE (u16)0

#define WAYLAND_XDG_WM_BASE_DESTROY_OPCODE (u16)0

#define WAYLAND_WL_SUBCOMPOSITOR_DESTROY_OPCODE (u16)0

#define COLOR_CHANNELS 4u

typedef struct wayland_context {
    u32 CurrentId;
    i32 SocketFd;

    u32 RegistryId;
    u32 WlCompositorId;
    u32 WlSurfaceId;

    u32 XdgWmBaseId;
    u32 XdgSurfaceId;
    u32 XdgToplevel;

    u32 Width;
    u32 Height;

    ring_buffer RingBuffer;

    //  state_state_t state;
} wayland_context;

typedef struct wayland_header {
    u32 ResourceId;
    u16 Opcode;
    u16 Size;
} wayland_header;

static void WaylandDisplayConnect(wayland_context *Context) {
    string8 XdgRuntimeDir = String8FromCstring(getenv("XDG_RUNTIME_DIR"));
    if (XdgRuntimeDir.Str == 0) Assert(0);

    struct sockaddr_un Sun = {.sun_family = AF_UNIX};

    Assert(XdgRuntimeDir.Size <= (sizeof(Sun.sun_path) - 1));
    u64 SocketPathLen = 0;

    MemoryCopy(Sun.sun_path, XdgRuntimeDir.Str, XdgRuntimeDir.Size);
    SocketPathLen = XdgRuntimeDir.Size;

    Sun.sun_path[SocketPathLen++] = '/';

    string8 WaylandDisplay = String8FromCstring(getenv("WAYLAND_DISPLAY"));
    if (WaylandDisplay.Str == 0) {
        WaylandDisplay = String8Lit("wayland-0");
    }
    MemoryCopy(Sun.sun_path + SocketPathLen, WaylandDisplay.Str, WaylandDisplay.Size);
    SocketPathLen += WaylandDisplay.Size;

    i32 Fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (Fd == -1) exit(errno);

    AssertAlways(connect(Fd, (struct sockaddr *)&Sun, sizeof(Sun)) != -1);
    Context->SocketFd = Fd;
    Context->CurrentId = 1;
}

static void WaylandGetRegistry(wayland_context *Context) {
    temp_arena Scratch = ScratchBegin(0, 0);

    u32 CurrentId = Context->CurrentId;
    i32 Fd = Context->SocketFd;

    CurrentId++;
    wayland_header Header = {WAYLAND_DISPLAY_OBJECT_ID, WAYLAND_WL_DISPLAY_GET_REGISTRY_OPCODE,
                             sizeof(wayland_header) + sizeof(CurrentId)};

    u8 *Buff = (u8 *)ArenaPush(Scratch.Arena, Header.Size);
    *(wayland_header *)Buff = Header;
    *(u32 *)(Buff + sizeof(wayland_header)) = CurrentId;

    AssertAlways(Header.Size == send(Fd, Buff, Header.Size, 0));

    Context->RegistryId = CurrentId;
    Context->CurrentId = CurrentId;
    ScratchEnd(Scratch);
}

static u32 WaylandRegistryBind(wayland_context *Context, u32 Name, u32 Version, char *Interface,
                               u32 InterfaceLen) {
    temp_arena Scratch = ScratchBegin(0, 0);

    u32 CurrentId = Context->CurrentId;
    i32 Fd = Context->SocketFd;
    wayland_header MessageHeader = {.ResourceId = Context->RegistryId,
                                    .Opcode = WAYLAND_WL_REGISTRY_BIND_OPCODE,
                                    .Size = sizeof(wayland_header) + sizeof(Name) + sizeof(InterfaceLen) +
                                            AlignPow2(InterfaceLen, 4) + sizeof(Version) + sizeof(CurrentId)};
    ++CurrentId;
    Assert(AlignPow2(MessageHeader.Size, 4) == MessageHeader.Size);

    u8 *Buff = (u8 *)ArenaPush(Scratch.Arena, MessageHeader.Size);
    u8 *Temp = Buff;

    *(wayland_header *)Buff = MessageHeader;
    Buff += sizeof(wayland_header);

    *(u32 *)Buff = Name;
    Buff += sizeof(Name);

    *(u32 *)Buff = InterfaceLen;
    Buff += sizeof(InterfaceLen);

    MemoryCopy(Buff, Interface, InterfaceLen);
    Buff += AlignPow2(InterfaceLen, 4);

    *(u32 *)Buff = Version;
    Buff += sizeof(Version);

    *(u32 *)Buff = CurrentId;
    Buff += sizeof(CurrentId);

    AssertAlways(send(Fd, Temp, MessageHeader.Size, MSG_DONTWAIT) == MessageHeader.Size);

    Context->CurrentId = CurrentId;
    ScratchEnd(Scratch);
    return CurrentId;
}

i32 main(void) {
    tctx Tctx;
    TctxInitAndEquip(&Tctx);

    AssertAlways(sizeof(wayland_header) == sizeof(wayland_header));
    wayland_context Context;

    Context.RingBuffer = RingBufferAlloc(KB(4));

    WaylandDisplayConnect(&Context);
    WaylandGetRegistry(&Context);

    ring_buffer RingBuffer = Context.RingBuffer;

    i64 Received = recv(Context.SocketFd, RingBuffer.Data + (RingBuffer.Written & (RingBuffer.RingSize - 1)),
                        RingBuffer.RingSize, 0);
    Assert(Received != -1);
    RingBuffer.Written += Received;
    WaylandLog("Received: %lld\n", Received);
    WaylandLog("Wayland Registry\n");
    while (RingBuffer.Written > RingBuffer.Read) {
        u64 ReadPrev = RingBuffer.Read;
        wayland_header Header = {0};
        RingBufferReadDirect(RingBuffer, Header, wayland_header);

        if (Header.ResourceId == Context.RegistryId && Header.Opcode == WAYLAND_WL_REGISTRY_EVENT_GLOBAL) {
            u32 Name = 0;
            RingBufferReadDirect(RingBuffer, Name, u32);

            u32 InterfaceLen = 0;
            RingBufferReadDirect(RingBuffer, InterfaceLen, u32);
            //        InterfaceLen = AlignPow2(InterfaceLen, 4);

            char *Interface = (char *)RingBuffer.Data + (RingBuffer.Read & (RingBuffer.RingSize - 1));
            RingBuffer.Read += AlignPow2(InterfaceLen, 4);

            u32 Version = 0;
            RingBufferReadDirect(RingBuffer, Version, u32);
            WaylandLog("  Name: %-3u Version: %-2u InterfaceLen: %-3u Interface: %-45s\n", Name, Version,
                       InterfaceLen, Interface);

            if (sizeof("xdg_wm_base") == InterfaceLen &&
                MemoryMatch(Interface, "xdg_wm_base", InterfaceLen)) {
                Context.XdgWmBaseId = WaylandRegistryBind(&Context, Name, Version, Interface, InterfaceLen);
            } else if (sizeof("wl_compositor") == InterfaceLen &&
                       MemoryMatch(Interface, "wl_compositor", InterfaceLen)) {
                Context.WlCompositorId =
                    WaylandRegistryBind(&Context, Name, Version, Interface, InterfaceLen);
            }
        } else if (Unlikely(Header.ResourceId == WAYLAND_DISPLAY_OBJECT_ID &&
                            Header.Opcode == WAYLAND_WL_DISPLAY_ERROR_EVENT)) {
            u32 ObjectId = 0;
            RingBufferReadDirect(RingBuffer, ObjectId, u32);

            u32 Code = 0;
            RingBufferReadDirect(RingBuffer, Code, u32);

            u32 ErrorLen = 0;
            RingBufferReadDirect(RingBuffer, ErrorLen, u32);
            WaylandLog("Fatal Error:\n  Target Obejct: %u Code: %u Error: %s\n\n", ObjectId, Code,
                       RingBuffer.Data + (RingBuffer.Read & (RingBuffer.RingSize - 1)));
        } else {
            Assert(0);
        }
        RingBuffer.Read = ReadPrev + Header.Size;
    }
    Context.RingBuffer = RingBuffer;

    AssertAlways(Context.XdgWmBaseId != 0 && Context.WlCompositorId != 0);

    {
        // NOTE(acol): Create surface
        temp_arena Scratch = ScratchBegin(0, 0);

        wayland_header Header = {.ResourceId = Context.WlCompositorId,
                                 .Opcode = WAYLAND_WL_COMPOSITOR_CREATE_SURFACE_OPCODE,
                                 .Size = sizeof(wayland_header) + sizeof(Context.CurrentId)};
        Context.CurrentId++;
        u8 *Buffer = (u8 *)ArenaPush(Scratch.Arena, Header.Size);
        u8 *Temp = Buffer;

        *(wayland_header *)Buffer = Header;
        Buffer += sizeof(wayland_header);

        *(u32 *)Buffer = Context.CurrentId;
        Buffer += sizeof(Context.CurrentId);

        Assert(Header.Size == send(Context.SocketFd, Temp, Buffer - Temp, 0));
        Context.WlSurfaceId = Context.CurrentId;

        ScratchEnd(Scratch);
    }

    {
        // NOTE(acol): Create XDG surface
        temp_arena Scratch = ScratchBegin(0, 0);

        wayland_header Header = {
            .ResourceId = Context.XdgWmBaseId,
            .Opcode = WAYLAND_XDG_WM_BASE_GET_XDG_SURFACE_OPCODE,
            .Size = sizeof(wayland_header) + sizeof(Context.CurrentId) + sizeof(Context.WlSurfaceId)};
        Context.CurrentId++;
        u8 *Buffer = (u8 *)ArenaPush(Scratch.Arena, Header.Size);
        u8 *Temp = Buffer;

        *(wayland_header *)Buffer = Header;
        Buffer += sizeof(wayland_header);

        *(u32 *)Buffer = Context.CurrentId;
        Buffer += sizeof(Context.CurrentId);

        *(u32 *)Buffer = Context.WlSurfaceId;
        Buffer += sizeof(Context.WlSurfaceId);

        Assert(Header.Size == send(Context.SocketFd, Temp, Buffer - Temp, 0));
        Context.XdgSurfaceId = Context.CurrentId;

        ScratchEnd(Scratch);
    }

    {
        // NOTE(acol): Create XDG top level surface
        temp_arena Scratch = ScratchBegin(0, 0);

        wayland_header Header = {.ResourceId = Context.XdgSurfaceId,
                                 .Opcode = WAYLAND_XDG_SURFACE_GET_TOPLEVEL_OPCODE,
                                 .Size = sizeof(wayland_header) + sizeof(Context.CurrentId)};
        Context.CurrentId++;
        u8 *Buffer = (u8 *)ArenaPush(Scratch.Arena, Header.Size);
        u8 *Temp = Buffer;

        *(wayland_header *)Buffer = Header;
        Buffer += sizeof(wayland_header);

        *(u32 *)Buffer = Context.CurrentId;
        Buffer += sizeof(Context.CurrentId);

        Assert(Header.Size == send(Context.SocketFd, Temp, Buffer - Temp, 0));
        Context.XdgToplevel = Context.CurrentId;

        ScratchEnd(Scratch);
    }

    {
        // NOTE(acol): Set title
        temp_arena Scratch = ScratchBegin(0, 0);

        string8 Title = String8Lit("Test title");

        wayland_header Header = {.ResourceId = Context.XdgToplevel,
                                 .Opcode = WAYLAND_XDG_TOPLEVEL_SET_TITLE_OPCODE,
                                 .Size = AlignPow2(sizeof(wayland_header) + sizeof(u32) + Title.Size + 1, 4)};

        u8 *Buffer = (u8 *)ArenaPush(Scratch.Arena, Header.Size);
        u8 *Temp = Buffer;

        *(wayland_header *)Buffer = Header;
        Buffer += sizeof(wayland_header);

        *(u32 *)Buffer = (u32)Title.Size + 1;
        Buffer += sizeof(u32);
        MemoryCopy(Buffer, Title.Str, Title.Size + 1);

        Assert(Header.Size == send(Context.SocketFd, Temp, Header.Size, 0));

        ScratchEnd(Scratch);
    }

    {
        // NOTE(acol): Commit surface
        temp_arena Scratch = ScratchBegin(0, 0);

        wayland_header Header = {.ResourceId = Context.WlSurfaceId,
                                 .Opcode = WAYLAND_WL_SURFACE_COMMIT_OPCODE,
                                 .Size = sizeof(wayland_header)};

        u8 *Buffer = (u8 *)ArenaPush(Scratch.Arena, Header.Size);

        *(wayland_header *)Buffer = Header;

        Assert(Header.Size == send(Context.SocketFd, Buffer, Header.Size, 0));

        ScratchEnd(Scratch);
    }
    {
        temp_arena Scratch = ScratchBegin(0, 0);
        ring_buffer RingBuffer = Context.RingBuffer;

        i64 Received =
            recv(Context.SocketFd, RingBuffer.Data + (RingBuffer.Written & (RingBuffer.RingSize - 1)),
                 RingBuffer.RingSize, 0);
        Assert(Received != -1);
        RingBuffer.Written += Received;
        WaylandLog("Received: %lld\n", Received);

        while (RingBuffer.Written > RingBuffer.Read) {
            u64 ReadPrev = RingBuffer.Read;
            wayland_header Header = {0};
            RingBufferReadDirect(RingBuffer, Header, wayland_header);
            if (Header.ResourceId == Context.XdgToplevel &&
                Header.Opcode == WAYLAND_XDG_TOPLEVEL_EVENT_CONFIGURE) {
                u32 Width = 0;
                RingBufferReadDirect(RingBuffer, Width, u32);

                u32 Height = 0;
                RingBufferReadDirect(RingBuffer, Height, u32);

                u32 Length = 0;
                RingBufferReadDirect(RingBuffer, Length, u32);

                u32 *States = (u32 *)ArenaPush(Scratch.Arena, Length);
                RingBufferRead(RingBuffer, States, Length);

                WaylandLog("Width=%u Height=%u", Width, Height);
                if (Length) {
                    WaylandLog(" states[ ");
                    for (i32 i = 0; i < Length / 4; i++) {
                        WaylandLog("%u ", States[i]);
                    }
                    WaylandLog("]");
                }
                WaylandLog("\n");
                if (Width && Height) {
                    Context.Width = Width;
                    Context.Height = Height;
                }
            } else if (Header.ResourceId == Context.XdgSurfaceId &&
                       Header.Opcode == WAYLAND_XDG_SURFACE_EVENT_CONFIGURE) {
                // NOTE(acol): Configure/Ack configure
                u32 Configure = 0;
                RingBufferReadDirect(RingBuffer, Configure, u32);
                WaylandLog("WAYLAND_XDG_SURFACE_EVENT_CONFIGURE\n");

                wayland_header SendHeader = {.ResourceId = Context.XdgSurfaceId,
                                             .Opcode = WAYLAND_XDG_SURFACE_ACK_CONFIGURE_OPCODE,
                                             .Size = sizeof(wayland_header) + sizeof(u32)};

                u8 *Buff = (u8 *)ArenaPush(Scratch.Arena, SendHeader.Size);
                *(wayland_header *)Buff = SendHeader;
                *(u32 *)(Buff + sizeof(wayland_header)) = Configure;
                Assert(SendHeader.Size == send(Context.SocketFd, Buff, SendHeader.Size, 0));
                WaylandLog("WAYLAND_XDG_SURFACE_ACK_CONFIGURE_OPCODE\n");

            } else if (Header.ResourceId == Context.XdgWmBaseId &&
                       Header.Opcode == WAYLAND_XDG_WM_BASE_EVENT_PING) {
                // NOTE(acol): Ping pong
                u32 Ping = 0;
                RingBufferReadDirect(RingBuffer, Ping, u32);
                WaylandLog("WAYLAND_XDG_WM_BASE_EVENT_PING\n");

                wayland_header SendHeader = {.ResourceId = Context.XdgWmBaseId,
                                             .Opcode = WAYLAND_XDG_WM_BASE_PONG_OPCODE,
                                             .Size = sizeof(wayland_header) + sizeof(u32)};

                u8 *Buff = (u8 *)ArenaPush(Scratch.Arena, SendHeader.Size);
                *(wayland_header *)Buff = SendHeader;
                *(u32 *)(Buff + sizeof(wayland_header)) = Ping;
                Assert(SendHeader.Size == send(Context.SocketFd, Buff, SendHeader.Size, 0));
                WaylandLog("WAYLAND_XDG_WM_BASE_PONG_OPCODE\n");

            } else if (Unlikely(Header.ResourceId == WAYLAND_DISPLAY_OBJECT_ID &&
                                Header.Opcode == WAYLAND_WL_DISPLAY_ERROR_EVENT)) {
                u32 ObjectId = 0;
                RingBufferReadDirect(RingBuffer, ObjectId, u32);

                u32 Code = 0;
                RingBufferReadDirect(RingBuffer, Code, u32);

                u32 ErrorLen = 0;
                RingBufferReadDirect(RingBuffer, ErrorLen, u32);
                WaylandLog(TXT_RED "\nFatal Error:\n  Target Obejct: %u Code: %u Error: %s\n\n" TXT_RST,
                           ObjectId, Code, RingBuffer.Data + (RingBuffer.Read & (RingBuffer.RingSize - 1)));
            } else {
                Trap();
            }
            RingBuffer.Read = ReadPrev + Header.Size;
        }
        ScratchEnd(Scratch);
        Context.RingBuffer = RingBuffer;
    }

    {
        temp_arena Scratch = ScratchBegin(0, 0);
        ring_buffer RingBuffer = Context.RingBuffer;

        i64 Received =
            recv(Context.SocketFd, RingBuffer.Data + (RingBuffer.Written & (RingBuffer.RingSize - 1)),
                 RingBuffer.RingSize, 0);
        Assert(Received != -1);
        RingBuffer.Written += Received;
        WaylandLog("Received: %lld\n", Received);

        while (RingBuffer.Written > RingBuffer.Read) {
            u64 ReadPrev = RingBuffer.Read;
            wayland_header Header = {0};
            RingBufferReadDirect(RingBuffer, Header, wayland_header);
            if (Header.ResourceId == Context.XdgWmBaseId && Header.Opcode == WAYLAND_XDG_WM_BASE_EVENT_PING) {
                // NOTE(acol): Ping pong
                u32 Ping = 0;
                RingBufferReadDirect(RingBuffer, Ping, u32);
                WaylandLog("WAYLAND_XDG_WM_BASE_EVENT_PING\n");

                wayland_header SendHeader = {.ResourceId = Context.XdgWmBaseId,
                                             .Opcode = WAYLAND_XDG_WM_BASE_PONG_OPCODE,
                                             .Size = sizeof(wayland_header) + sizeof(u32)};

                u8 *Buff = (u8 *)ArenaPush(Scratch.Arena, SendHeader.Size);
                *(wayland_header *)Buff = SendHeader;
                *(u32 *)(Buff + sizeof(wayland_header)) = Ping;
                Assert(SendHeader.Size == send(Context.SocketFd, Buff, SendHeader.Size, 0));
                WaylandLog("WAYLAND_XDG_WM_BASE_PONG_OPCODE\n");
            } else if (Unlikely(Header.ResourceId == WAYLAND_DISPLAY_OBJECT_ID &&
                                Header.Opcode == WAYLAND_WL_DISPLAY_ERROR_EVENT)) {
                u32 ObjectId = 0;
                RingBufferReadDirect(RingBuffer, ObjectId, u32);

                u32 Code = 0;
                RingBufferReadDirect(RingBuffer, Code, u32);

                u32 ErrorLen = 0;
                RingBufferReadDirect(RingBuffer, ErrorLen, u32);
                WaylandLog("Fatal Error:\n  Target Obejct: %u Code: %u Error: %s\n\n", ObjectId, Code,
                           RingBuffer.Data + (RingBuffer.Read & (RingBuffer.RingSize - 1)));
            }

            RingBuffer.Read = ReadPrev + Header.Size;
        }
        ScratchEnd(Scratch);
    }

    // {
    //     // NOTE(acol): cleanup
    //     temp_arena Scratch = ScratchBegin(0, 0);
    //
    //     wayland_header Header = {.ResourceId = Context.XdgToplevel,
    //                              .Opcode = WAYLAND_XDG_TOPLEVEL_DESTROY_OPCODE,
    //                              .Size = sizeof(wayland_header)};
    //
    //     u8 *Buffer = (u8 *)ArenaPush(Scratch.Arena, Header.Size);
    //     *(wayland_header *)Buffer = Header;
    //
    //     Assert(Header.Size == send(Context.SocketFd, Buffer, Header.Size, 0));
    //
    //     Header.ResourceId = Context.XdgSurfaceId;
    //     Header.Opcode = WAYLAND_XDG_SURFACE_DESTROY_OPCODE;
    //     *(wayland_header *)Buffer = Header;
    //     Assert(Header.Size == send(Context.SocketFd, Buffer, Header.Size, 0));
    //
    //     Header.ResourceId = Context.WlSurfaceId;
    //     Header.Opcode = WAYLAND_WL_SURFACE_DESTROY_OPCODE;
    //     *(wayland_header *)Buffer = Header;
    //     Assert(Header.Size == send(Context.SocketFd, Buffer, Header.Size, 0));
    //
    //     Header.ResourceId = Context.XdgWmBaseId;
    //     Header.Opcode = WAYLAND_XDG_WM_BASE_DESTROY_OPCODE;
    //     *(wayland_header *)Buffer = Header;
    //     Assert(Header.Size == send(Context.SocketFd, Buffer, Header.Size, 0));
    //
    //     Header.ResourceId = Context.WlCompositorId;
    //     Header.Opcode = WAYLAND_WL_SUBCOMPOSITOR_DESTROY_OPCODE;
    //     *(wayland_header *)Buffer = Header;
    //     Assert(Header.Size == send(Context.SocketFd, Buffer, Header.Size, 0));
    //
    //     ScratchEnd(Scratch);
    // }

    RingBufferRelease(Context.RingBuffer);
    TctxRelease();
    printf("Made it\n");
    return 0;
}
