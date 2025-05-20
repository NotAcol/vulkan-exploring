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
        string8 WaylandDisplay = String8Lit("wayland-0");
    }
    MemoryCopy(Sun.sun_path + SocketPathLen, WaylandDisplay.Str, WaylandDisplay.Size);
    SocketPathLen += WaylandDisplay.Size;

    i32 Fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (Fd == -1) exit(errno);

    if (connect(Fd, (struct sockaddr *)&Sun, sizeof(Sun)) == -1) exit(errno);

    Context->SocketFd = Fd;
    Context->CurrentId = 1;
}

static wayland_message WaylandGetRegistry(wayland_context *Context) {
    temp_arena Scratch = ScratchBegin(0, 0);

    u32 CurrentId = Context->CurrentId;
    i32 Fd = Context->SocketFd;

    CurrentId++;

    wayland_header Header = {WAYLAND_DISPLAY_OBJECT_ID, WAYLAND_WL_DISPLAY_GET_REGISTRY_OPCODE,
                             sizeof(wayland_header) + sizeof(Context->CurrentId)};

    u8 *Buff = (u8 *)ArenaPush(Scratch.Arena, sizeof(wayland_header) + sizeof(Context->CurrentId));
    *(wayland_header *)Buff = Header;
    *(u32 *)(Buff + sizeof(wayland_header)) = CurrentId;

    Assert((sizeof(wayland_header) + sizeof(CurrentId)) ==
           send(Fd, Buff, sizeof(wayland_header) + sizeof(CurrentId), MSG_DONTWAIT));

    Context->RegistryId = CurrentId;
    Context->CurrentId = CurrentId;
    ScratchEnd(Scratch);
}

static void WaylandReadMessages(wayland_context *Context) {
    ring_buffer RingBuffer = Context->RingBuffer;

    i64 Read = recv(Context->SocketFd, RingBuffer.Data + (RingBuffer.Read & (RingBuffer.RingSize - 1)),
                    RingBuffer.RingSize - (RingBuffer.Written - RingBuffer.Read), 0);
    Assert(Read != -1);
    RingBuffer.Written += Read;
}

static void WaylandHandleEvents(wayland_context *Context) {
    WaylandReadMessages(Context);
    ring_buffer RingBuffer = Context->RingBuffer;

    while (RingBiffer.Read < RingBuffer.Written) {
        wayland_header Header = *(wayland_header *)(RingBuffer.Data + RingBuffer.Read);
        u8 *Data = RingBuffer.Data + RingBuffer.Read + Header.Size - sizeof(wayland_header);
        switch (Header.ResourceId) {
            case Context->RegistryId: {
                if (Header.Opcode == WAYLAND_WL_REGISTRY_EVENT_GLOBAL) {
                    if (srtcmp()) }
            } break;
        }
    }
}
