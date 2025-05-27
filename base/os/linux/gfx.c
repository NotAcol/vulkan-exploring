static u32 WaylandRegistryBind(u32 Fd, u32 CurrentId, u32 Registry, u32 Name, u32 Version, u32 InterfaceLen,
                               char *Interface) {
    temp_arena Scratch = ScratchBegin(0, 0);
    wayland_header Header = {.ObjectId = Registry,
                             .OpCode = WL_REGISTRY_BIND_OPCODE,
                             .Size = sizeof(wayland_header) + sizeof(Name) + sizeof(InterfaceLen) +
                                     AlignPow2(InterfaceLen, 4) + sizeof(Version) + sizeof(CurrentId)};
    Assert(AlignPow2(Header.Size, 4) == Header.Size);

    u8 *Buff = (u8 *)ArenaPush(Scratch.Arena, Header.Size);
    u8 *Temp = Buff;

    *(wayland_header *)Buff = Header;
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

    AssertAlways(send(Fd, Temp, Header.Size, MSG_DONTWAIT) == Header.Size);

    ScratchEnd(Scratch);
    return CurrentId;
}

static void WaylandInit(wayland_context *WlCtx, u32 Height, u32 Width, string8 Title) {
    temp_arena Scratch = ScratchBegin(0, 0);
    ring_buffer RingBuffer = RingBufferAlloc(KB(4));

    /*  =====================================================================================

                                    NOTE(acol): Connect to wayland display

        ===================================================================================== */
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
    u32 CurrentId = 1;

    WaylandLog("Connected to wayland server\n");

    /*  =====================================================================================

                            NOTE(acol): Get registry and global objects

        ===================================================================================== */
    CurrentId++;
    wayland_header Header = {.ObjectId = DISPLAY_OBJECT_ID,
                             .OpCode = WL_DISPLAY_GET_REGISTRY_OPCODE,
                             .Size = sizeof(wayland_header) + sizeof(CurrentId)};
    u8 *Buff = (u8 *)ArenaPush(Scratch.Arena, Header.Size);
    *(wayland_header *)Buff = Header;
    *(u32 *)(Buff + sizeof(wayland_header)) = CurrentId;

    Assert(Header.Size == send(Fd, Buff, Header.Size, 0));
    u32 Registry = CurrentId;

    u32 XdgWmBase;
    u32 WlCompositor;

    i64 Received =
        recv(Fd, RingBuffer.Data + (RingBuffer.Written & (RingBuffer.RingSize - 1)), RingBuffer.RingSize, 0);
    Assert(Received != -1);

    RingBuffer.Written += Received;
    WaylandLog("\nWayland Received: %lldB\n", Received);
    WaylandLog("Wayland Registry\n");
    while (RingBuffer.Written > RingBuffer.Read) {
        u64 ReadPrev = RingBuffer.Read;
        RingBufferReadDirect(RingBuffer, Header, wayland_header);
        if (Header.ObjectId == Registry && Header.OpCode == WL_REGISTRY_EVENT_GLOBAL) {
            u32 Name = 0;
            RingBufferReadDirect(RingBuffer, Name, u32);

            u32 InterfaceLen = 0;
            RingBufferReadDirect(RingBuffer, InterfaceLen, u32);

            char *Interface = (char *)RingBuffer.Data + (RingBuffer.Read & (RingBuffer.RingSize - 1));
            RingBuffer.Read += AlignPow2(InterfaceLen, 4);

            u32 Version = 0;
            RingBufferReadDirect(RingBuffer, Version, u32);
            WaylandLog("  Name: %-3u Version: %-2u InterfaceLen: %-3u Interface: %-45s\n", Name, Version,
                       InterfaceLen, Interface);

            if (sizeof("xdg_wm_base") == InterfaceLen &&
                MemoryMatch(Interface, "xdg_wm_base", InterfaceLen)) {
                CurrentId++;
                XdgWmBase =
                    WaylandRegistryBind(Fd, CurrentId, Registry, Name, Version, InterfaceLen, Interface);
            } else if (sizeof("wl_compositor") == InterfaceLen &&
                       MemoryMatch(Interface, "wl_compositor", InterfaceLen)) {
                CurrentId++;
                WlCompositor =
                    WaylandRegistryBind(Fd, CurrentId, Registry, Name, Version, InterfaceLen, Interface);
            }
        } else if (Unlikely(Header.ObjectId == DISPLAY_OBJECT_ID &&
                            Header.OpCode == WL_DISPLAY_ERROR_EVENT)) {
            u32 ObjectId = 0;
            RingBufferReadDirect(RingBuffer, ObjectId, u32);

            u32 Code = 0;
            RingBufferReadDirect(RingBuffer, Code, u32);

            u32 ErrorLen = 0;
            RingBufferReadDirect(RingBuffer, ErrorLen, u32);
            WaylandLog("Fatal Error:\n  Target Obejct: %u Code: %u Error: %s\n\n", ObjectId, Code,
                       RingBuffer.Data + (RingBuffer.Read & (RingBuffer.RingSize - 1)));
            Trap();
        } else {
            Trap();
        }
        RingBuffer.Read = ReadPrev + Header.Size;
    }
    AssertAlways(XdgWmBase != 0 && WlCompositor != 0);

    /*  =====================================================================================

                                    NOTE(acol): Create surface

        ===================================================================================== */
    Header = (wayland_header){.ObjectId = WlCompositor,
                              .OpCode = WL_COMPOSITOR_CREATE_SURFACE_OPCODE,
                              .Size = sizeof(wayland_header) + sizeof(CurrentId)};
    CurrentId++;
    Buff = (u8 *)ArenaPush(Scratch.Arena, Header.Size);
    u8 *Temp = Buff;

    *(wayland_header *)Buff = Header;
    Buff += sizeof(wayland_header);

    *(u32 *)Buff = CurrentId;
    Buff += sizeof(CurrentId);

    Buff = Temp;
    Assert(Header.Size == send(Fd, Buff, Header.Size, MSG_DONTWAIT));
    u32 WlSurface = CurrentId;

    /*  =====================================================================================

                                    NOTE(acol): Create XDG surface

        ===================================================================================== */
    Header = (wayland_header){.ObjectId = XdgWmBase,
                              .OpCode = XDG_WM_BASE_GET_XDG_SURFACE_OPCODE,
                              .Size = sizeof(wayland_header) + sizeof(CurrentId) + sizeof(WlSurface)};
    CurrentId++;
    Buff = (u8 *)ArenaPush(Scratch.Arena, Header.Size);
    Temp = Buff;

    *(wayland_header *)Buff = Header;
    Buff += sizeof(wayland_header);

    *(u32 *)Buff = CurrentId;
    Buff += sizeof(CurrentId);

    *(u32 *)Buff = WlSurface;
    Buff += sizeof(WlSurface);

    Buff = Temp;
    Assert(Header.Size == send(Fd, Buff, Header.Size, MSG_DONTWAIT));
    u32 XdgSurface = CurrentId;

    /*  =====================================================================================

                                    NOTE(acol): Create XDG top level surface

        ===================================================================================== */
    Header = (wayland_header){.ObjectId = XdgSurface,
                              .OpCode = XDG_SURFACE_GET_TOPLEVEL_OPCODE,
                              .Size = sizeof(wayland_header) + sizeof(CurrentId)};
    CurrentId++;
    Buff = (u8 *)ArenaPush(Scratch.Arena, Header.Size);
    Temp = Buff;

    *(wayland_header *)Buff = Header;
    Buff += sizeof(wayland_header);

    *(u32 *)Buff = CurrentId;
    Buff += sizeof(CurrentId);

    Buff = Temp;
    Assert(Header.Size == send(Fd, Buff, Header.Size, MSG_DONTWAIT));
    u32 XdgToplevel = CurrentId;

    /*  =====================================================================================

                                    NOTE(acol): Set title

        ===================================================================================== */
    Header = (wayland_header){.ObjectId = XdgToplevel,
                              .OpCode = XDG_TOPLEVEL_SET_TITLE_OPCODE,
                              .Size = AlignPow2(sizeof(wayland_header) + sizeof(u32) + Title.Size + 1, 4)};
    Buff = (u8 *)ArenaPush(Scratch.Arena, Header.Size);
    Temp = Buff;

    *(wayland_header *)Buff = Header;
    Buff += sizeof(wayland_header);

    *(u32 *)Buff = (u32)Title.Size + 1;
    Buff += sizeof(u32);
    MemoryCopy(Buff, Title.Str, Title.Size + 1);

    Buff = Temp;
    Assert(Header.Size == send(Fd, Buff, Header.Size, MSG_DONTWAIT));

    /*  =====================================================================================

                                    NOTE(acol): Commit surface

        ===================================================================================== */
    Header = (wayland_header){
        .ObjectId = WlSurface, .OpCode = WL_SURFACE_COMMIT_OPCODE, .Size = sizeof(wayland_header)};

    Buff = (u8 *)ArenaPush(Scratch.Arena, Header.Size);

    *(wayland_header *)Buff = Header;

    Assert(Header.Size == send(Fd, Buff, Header.Size, MSG_DONTWAIT));

    /*  =====================================================================================

                                    NOTE(acol): Read events

        ===================================================================================== */
    Received =
        recv(Fd, RingBuffer.Data + (RingBuffer.Written & (RingBuffer.RingSize - 1)), RingBuffer.RingSize, 0);
    Assert(Received != -1);

    RingBuffer.Written += Received;
    WaylandLog("\nWayland Received: %lld\n", Received);

    while (RingBuffer.Written > RingBuffer.Read) {
        u64 ReadPrev = RingBuffer.Read;
        wayland_header Header = {0};
        RingBufferReadDirect(RingBuffer, Header, wayland_header);
        if (Header.ObjectId == XdgToplevel && Header.OpCode == XDG_TOPLEVEL_EVENT_CONFIGURE) {
#if WAYLAND_DEBUG
    #define X(S) #S,
            char *StateNames[] = {TOPLEVEL_STATES};
    #undef X
#endif
            u32 NewWidth = 0;
            RingBufferReadDirect(RingBuffer, NewWidth, u32);

            u32 NewHeight = 0;
            RingBufferReadDirect(RingBuffer, NewHeight, u32);

            u32 Length = 0;
            RingBufferReadDirect(RingBuffer, Length, u32);

            u32 *States = (u32 *)ArenaPush(Scratch.Arena, Length);
            RingBufferRead(RingBuffer, States, Length);

            WaylandLog("Width=%u Height=%u", NewWidth, NewHeight);
            if (Length) {
                WaylandLog(" States[ ");
                for (u32 i = 0; i < Length / 4; i++) {
                    WaylandLog("%s, ", StateNames[States[i] - 1]);
                }
                WaylandLog("]");
            }
            WaylandLog("\n");
            if (NewWidth && NewHeight) {
                Width = NewWidth;
                Height = NewHeight;
            }
        } else if (Header.ObjectId == XdgSurface && Header.OpCode == XDG_SURFACE_EVENT_CONFIGURE) {
            // NOTE(acol): Configure/Ack configure
            u32 Configure = 0;
            RingBufferReadDirect(RingBuffer, Configure, u32);
            WaylandLog("WAYLAND_XDG_SURFACE_EVENT_CONFIGURE\n");

            wayland_header SendHeader = {.ObjectId = XdgSurface,
                                         .OpCode = XDG_SURFACE_ACK_CONFIGURE_OPCODE,
                                         .Size = sizeof(wayland_header) + sizeof(u32)};

            u8 *Buff = (u8 *)ArenaPush(Scratch.Arena, SendHeader.Size);
            *(wayland_header *)Buff = SendHeader;
            *(u32 *)(Buff + sizeof(wayland_header)) = Configure;
            Assert(SendHeader.Size == send(Fd, Buff, SendHeader.Size, 0));
            WaylandLog("WAYLAND_XDG_SURFACE_ACK_CONFIGURE_OPCODE\n");

        } else if (Header.ObjectId == XdgToplevel && Header.OpCode == XDG_TOPLEVEL_EVENT_CONFIGURE_BOUNDS) {
            WaylandLog("XDG_TOPLEVEL_EVENT_CONFIGURE_BOUNDS\n");
        } else if (Header.ObjectId == XdgToplevel && Header.OpCode == XDG_TOPLEVEL_EVENT_WM_CAPABILITIES) {
#if WAYLAND_DEBUG
    #define X(S) #S,
            char *CapabilitiesNames[] = {TOPLEVEL_WM_CAPABILITIES};
    #undef X
#endif
            u32 Length = 0;
            RingBufferReadDirect(RingBuffer, Length, u32);

            u32 *Capabilities = (u32 *)ArenaPush(Scratch.Arena, Length);
            RingBufferRead(RingBuffer, Capabilities, Length);

            WaylandLog("Wayland Window Manager Capabilities[ ");
            for (u32 i = 0; i < Length / 4; i++) {
                WaylandLog("%s, ", CapabilitiesNames[Capabilities[i] - 1]);
            }
            WaylandLog(" ]\n");

        } else if (Header.ObjectId == XdgToplevel && Header.OpCode == XDG_TOPLEVEL_EVENT_CLOSE) {
            Trap();
        } else if (Unlikely(Header.ObjectId == DISPLAY_OBJECT_ID &&
                            Header.OpCode == WL_DISPLAY_ERROR_EVENT)) {
            u32 ObjectId = 0;
            RingBufferReadDirect(RingBuffer, ObjectId, u32);

            u32 Code = 0;
            RingBufferReadDirect(RingBuffer, Code, u32);

            u32 ErrorLen = 0;
            RingBufferReadDirect(RingBuffer, ErrorLen, u32);
            WaylandLog(TXT_RED "\nFatal Error:\n  Target Obejct: %u Code: %u Error: %s\n\n" TXT_RST, ObjectId,
                       Code, RingBuffer.Data + (RingBuffer.Read & (RingBuffer.RingSize - 1)));
            Trap();
        } else {
            Trap();
        }
        RingBuffer.Read = ReadPrev + Header.Size;
    }

    /*  =====================================================================================

                                    NOTE(acol): Commit surface

        ===================================================================================== */
    Header = (wayland_header){
        .ObjectId = WlSurface, .OpCode = WL_SURFACE_COMMIT_OPCODE, .Size = sizeof(wayland_header)};

    Buff = (u8 *)ArenaPush(Scratch.Arena, Header.Size);

    *(wayland_header *)Buff = Header;

    Assert(Header.Size == send(Fd, Buff, Header.Size, MSG_DONTWAIT));

    WaylandLog(TXT_UYEL "\nWayland Id\n" TXT_WHT
                        "  Display: %u\n  Registry: %u\n  WlCompositor: %u\n  WlSurface: %u\n  "
                        "XdgWmBase: %u\n  XdgSurface: %u\n  XdgToplevel: %u\n\n" TXT_RST,
               DISPLAY_OBJECT_ID, Registry, WlCompositor, WlSurface, XdgWmBase, XdgSurface, XdgToplevel);

    // NOTE(acol): Write back
    WlCtx->Width = Width;
    WlCtx->Height = Height;
    WlCtx->XdgToplevel = XdgToplevel;
    WlCtx->XdgSurface = XdgSurface;
    WlCtx->XdgWmBase = XdgWmBase;
    WlCtx->WlSurface = WlSurface;
    WlCtx->WlCompositor = WlCompositor;
    WlCtx->Registry = Registry;
    WlCtx->CurrentId = CurrentId;
    WlCtx->Fd = Fd;
    WlCtx->RingBuffer = RingBuffer;
    WlCtx->State = WLSTATE_SurfaceCommited;

    ScratchEnd(Scratch);
}

static void WaylandCommitSurface(wayland_context *WlCtx) {
    temp_arena Scratch = ScratchBegin(0, 0);
    wayland_header Header = {
        .ObjectId = WlCtx->WlSurface, .OpCode = WL_SURFACE_COMMIT_OPCODE, .Size = sizeof(wayland_header)};

    u8 *Buff = (u8 *)ArenaPush(Scratch.Arena, Header.Size);

    *(wayland_header *)Buff = Header;

    Assert(Header.Size == send(WlCtx->Fd, Buff, Header.Size, MSG_DONTWAIT));
    ScratchEnd(Scratch);
}

static void WaylandPollEvents(wayland_context *WlCtx) {
    temp_arena Scratch = ScratchBegin(0, 0);
    ring_buffer RingBuffer = WlCtx->RingBuffer;
    u32 Height = WlCtx->Height;
    u32 Width = WlCtx->Width;
    i32 Fd = WlCtx->Fd;
    u32 XdgSurface = WlCtx->XdgSurface;
    u32 XdgToplevel = WlCtx->XdgToplevel;
    wayland_state State = WlCtx->State;
    b32 ShouldQuit;

    if (State == WLSTATE_Quit || State == WLSTATE_Error) return;

    struct pollfd Pfds[] = {{
        .fd = Fd,
        .events = POLLIN,
    }};

    i32 HasEvents = poll(Pfds, 1, 0);
    Assert(HasEvents != -1);
    for (i32 i = 0; i < HasEvents; i++) {
        i32 Fd = Pfds[i].fd;
        i32 Received = recv(Fd, RingBuffer.Data + (RingBuffer.Written & (RingBuffer.RingSize - 1)),
                            RingBuffer.RingSize, 0);
        Assert(Received != -1);
        RingBuffer.Written += Received;
        WaylandLog("\nWayland Received: %d\n", Received);

        while ((RingBuffer.Written > RingBuffer.Read) && (State != WLSTATE_Error)) {
            u64 ReadPrev = RingBuffer.Read;
            wayland_header Header = {0};
            RingBufferReadDirect(RingBuffer, Header, wayland_header);

            RingBuffer.Read = ReadPrev + Header.Size;

            if (Header.ObjectId == XdgToplevel && Header.OpCode == XDG_TOPLEVEL_EVENT_CONFIGURE) {
#if WAYLAND_DEBUG
    #define X(S) #S,
                char *StateNames[] = {TOPLEVEL_STATES};
    #undef X
#endif
                u32 NewWidth = 0;
                RingBufferReadDirect(RingBuffer, NewWidth, u32);

                u32 NewHeight = 0;
                RingBufferReadDirect(RingBuffer, NewHeight, u32);

                u32 Length = 0;
                RingBufferReadDirect(RingBuffer, Length, u32);

                u32 *States = (u32 *)ArenaPush(Scratch.Arena, Length);
                RingBufferRead(RingBuffer, States, Length);

                WaylandLog("Width=%u Height=%u", NewWidth, NewHeight);
                if (Length) {
                    WaylandLog(" States[ ");
                    for (u32 i = 0; i < Length / 4; i++) {
                        WaylandLog("%s, ", StateNames[States[i] - 1]);
                    }
                    WaylandLog("]");
                }
                WaylandLog("\n");
                if (NewWidth && NewHeight) {
                    Width = NewWidth;
                    Height = NewHeight;
                    State = WLSTATE_NewSize;
                }
            } else if (Header.ObjectId == XdgToplevel &&
                       Header.OpCode == XDG_TOPLEVEL_EVENT_CONFIGURE_BOUNDS) {
                WaylandLog("XDG_TOPLEVEL_EVENT_CONFIGURE_BOUNDS\n");
            } else if (Header.ObjectId == XdgSurface && Header.OpCode == XDG_SURFACE_EVENT_CONFIGURE) {
                // NOTE(acol): Configure/Ack configure
                u32 Configure = 0;
                RingBufferReadDirect(RingBuffer, Configure, u32);
                WaylandLog("WAYLAND_XDG_SURFACE_EVENT_CONFIGURE\n");

                wayland_header SendHeader = {.ObjectId = XdgSurface,
                                             .OpCode = XDG_SURFACE_ACK_CONFIGURE_OPCODE,
                                             .Size = sizeof(wayland_header) + sizeof(u32)};

                u8 *Buff = (u8 *)ArenaPush(Scratch.Arena, SendHeader.Size);
                *(wayland_header *)Buff = SendHeader;
                *(u32 *)(Buff + sizeof(wayland_header)) = Configure;
                Assert(SendHeader.Size == send(Fd, Buff, SendHeader.Size, 0));
                WaylandLog("WAYLAND_XDG_SURFACE_ACK_CONFIGURE_OPCODE\n");
                if (State == WLSTATE_NewSize) State = WLSTATE_ReadyToResize;
            } else if (Header.ObjectId == XdgToplevel && Header.OpCode == XDG_TOPLEVEL_EVENT_CLOSE) {
                ShouldQuit = 1;
            } else if (Unlikely(Header.ObjectId == DISPLAY_OBJECT_ID &&
                                Header.OpCode == WL_DISPLAY_ERROR_EVENT)) {
                u32 ObjectId = 0;
                RingBufferReadDirect(RingBuffer, ObjectId, u32);

                u32 Code = 0;
                RingBufferReadDirect(RingBuffer, Code, u32);

                u32 ErrorLen = 0;
                RingBufferReadDirect(RingBuffer, ErrorLen, u32);
                WaylandLog(TXT_RED "\nFatal Error:\n  Target Obejct: %u Code: %u Error: %s\n\n" TXT_RST,
                           ObjectId, Code, RingBuffer.Data + (RingBuffer.Read & (RingBuffer.RingSize - 1)));
                State = WLSTATE_Error;
                Trap();
            } else {
                WaylandLog("Unknown Event: ObjectId: %u, OpCode: %u, Size: %u\n", Header.ObjectId,
                           Header.OpCode, Header.Size);
            }
            RingBuffer.Read = ReadPrev + Header.Size;
        }
    }

    WlCtx->Height = Height;
    WlCtx->Width = Width;
    WlCtx->RingBuffer = RingBuffer;
    if (ShouldQuit && (State != WLSTATE_Error)) {
        WlCtx->State = WLSTATE_Quit;
    }

    ScratchEnd(Scratch);
}

static b32 WaylandShouldResize(wayland_context *WlCtx) {
    if (WlCtx->State == WLSTATE_ReadyToResize) return 1;
    return 0;
}
