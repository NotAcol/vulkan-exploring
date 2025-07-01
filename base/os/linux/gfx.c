#define WaylandSerialize(Buffer, Item)  \
    *(__typeof__(Item) *)Buffer = Item; \
    Buffer                      = Buffer + sizeof(Item)

#define WaylandReadHelper(Buffer, Item, At)          \
    Item = *(__typeof__(Item) *)((u8 *)Buffer + At); \
    At += sizeof(Item)

static void *WaylandGetMessages(i32 Fd, ring_buffer *RingBuffer) {
    void *Ret = RingBuffer->Data + (RingBuffer->Written & (RingBuffer->RingSize - 1));

    i64 Received = recv(Fd, Ret, RingBuffer->RingSize, 0);

    Assert(Received != -1);
    if (Received == -1) return (void *)0;

    RingBuffer->Written += Received;

    WaylandLog("\nWayland Received: %lldB\n", Received);

    return Ret;
}

static void WaylandDisplayGetRegistry(i32 Fd, u32 CurrentId) {
    temp_arena Scratch = ScratchBegin(0, 0);

    wayland_header Header = {
        .ObjectId = WL_DISPLAY_ID, .OpCode = 1, .Size = sizeof(wayland_header) + sizeof(CurrentId)};

    u8 *Buff = (u8 *)ArenaPush(Scratch.Arena, Header.Size);
    u8 *Temp = Buff;

    WaylandSerialize(Temp, Header);
    WaylandSerialize(Temp, CurrentId);

    u64 Sent = send(Fd, Buff, Header.Size, MSG_DONTWAIT);
    Assert(Header.Size == Sent);
    (void)Sent;

    WaylandLog("Wayland: Sent wl_display::get_registry\n");

    ScratchEnd(Scratch);
}

static void WaylandRegistryBind(i32 Fd, u32 RegistryId, u32 Name, string8 Interface, u32 Version, u32 Id) {
    temp_arena Scratch = ScratchBegin(0, 0);

    u32 Size = Interface.Size + 1;

    wayland_header Header = {.ObjectId = RegistryId,
                             .OpCode   = 0,
                             .Size     = sizeof(wayland_header) + sizeof(Name) + sizeof(Size) +
                                     AlignPow2(Size, 4) + sizeof(Version) + sizeof(Id)};
    Assert(Header.Size == AlignPow2(Header.Size,4));

    printf("%lu %lu %lu %u %lu %lu \n", sizeof(wayland_header), sizeof(Name), sizeof(Size),
           AlignPow2(Size, 4), sizeof(Version), sizeof(Id));

    u8 *Buff = (u8 *)ArenaPush(Scratch.Arena, Header.Size);
    u8 *Temp = Buff;

    WaylandSerialize(Temp, Header);
    WaylandSerialize(Temp, Name);
    WaylandSerialize(Temp, Size);

    MemoryCopy(Temp, Interface.Str, Size);
    Temp += Size;

    WaylandSerialize(Temp, Version);
    WaylandSerialize(Temp, Id);

    u64 Sent = send(Fd, Buff, Header.Size, MSG_DONTWAIT);
    Assert(Header.Size == Sent);
    (void)Sent;

    WaylandLog("Wayland: Sent wl_registry::bind %s\n", Interface.Str);

    ScratchEnd(Scratch);
}

static void WaylandInit(wayland_context *WlCtx, u32 Height, u32 Width, string8 Title) {
    // temp_arena Scratch     = ScratchBegin(0, 0);
    ring_buffer RingBuffer = RingBufferAlloc(KB(4));

    string8 XdgRuntimeDir = String8FromCstring(getenv("XDG_RUNTIME_DIR"));
    if (XdgRuntimeDir.Str == 0) Assert(0);

    struct sockaddr_un Sun = {.sun_family = AF_UNIX};

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
    u32 CurrentId = 2;

    WaylandLog("Connected to wayland server\n");

    WaylandDisplayGetRegistry(Fd, CurrentId);
    u32 Registry = CurrentId;
    u32 XdgWmBase;
    u32 WlCompositor;

    WaylandGetMessages(Fd, &RingBuffer);

    WaylandLog("Wayland Registry\n");
    while (RingBuffer.Read < RingBuffer.Written) {
        u64 At                = 0;
        u8 *Buff              = RingBufferReadPtr(&RingBuffer);
        wayland_header Header = {0};

        WaylandReadHelper(Buff, Header, At);

        if (Header.ObjectId == Registry && Header.OpCode == WL_REGISTRY_EVENT_GLOBAL) {
            u32 Name          = 0;
            string8 Interface = {0};
            u32 Version       = 0;

            WaylandReadHelper(Buff, Name, At);

            Interface.Size = *(u32 *)(Buff + At);
            At += sizeof(u32);
            Interface.Str = Buff + At;
            At += AlignPow2(Interface.Size, 4);

            WaylandReadHelper(Buff, Version, At);

            Assert(At == Header.Size);
            WaylandLog("  Name: %-3u Version: %-2u InterfaceLen: %-3llu Interface: %-45s\n", Name, Version,
                       Interface.Size, Interface.Str);

            if (sizeof("xdg_wm_base") == Interface.Size &&
                MemoryMatch(Interface.Str, "xdg_wm_base", Interface.Size)) {
                CurrentId++;
                WaylandRegistryBind(Fd, Registry, Name, String8Lit("xdg_wm_base"), Version, CurrentId);
                XdgWmBase = CurrentId;
            } else if (sizeof("wl_compositor") == Interface.Size &&
                       MemoryMatch(Interface.Str, "wl_compositor", Interface.Size)) {
                CurrentId++;
                WaylandRegistryBind(Fd, Registry, Name, String8Lit("wl_compositor"), Version, CurrentId);
                WlCompositor = CurrentId;
            }
        } else if (Unlikely(Header.ObjectId == WL_DISPLAY_ID && Header.OpCode == WL_DISPLAY_EVENT_ERROR)) {
            u32 ObjectId  = 0;
            u32 Code      = 0;
            string8 Error = {0};

            WaylandReadHelper(Buff, ObjectId, At);
            WaylandReadHelper(Buff, Code, At);

            Error.Size = *(u32 *)(Buff + At);
            At += sizeof(u32);
            Error.Str = Buff + At;
            At += AlignPow2(Error.Size, 4);

            WaylandLog(TXT_RED "\nFatal Error:\n" TXT_RST "  Target Obejct: %u\n  Code: %u\n  Error: %s\n\n",
                       ObjectId, Code, Error.Str);
            Trap();
        } else {
            Trap();
        }
        RingBufferReadEnd(&RingBuffer, Header.Size);
    }

    // ScratchEnd(Scratch);
}
