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

    Trap();

    CurrentId++;
    u64 Pos = 0;
    u8 *Buff = ArenaPush(Scratch.Arena, WAYLAND_HEADER_SIZE + sizeof(Context->CurrentId));

    WaylandMessagePush(Buff, Pos, WAYLAND_DISPLAY_OBJECT_ID, u32);
    WaylandMessagePush(Buff, Pos, WAYLAND_WL_DISPLAY_GET_REGISTRY_OPCODE, u16);
    WaylandMessagePush(Buff, Pos, WAYLAND_HEADER_SIZE + sizeof(Context->CurrentId), u16);
    WaylandMessagePush(Buff, Pos, CurrentId, u32);

    if ((WAYLAND_HEADER_SIZE + sizeof(CurrentId)) !=
        send(Fd, Buff, WAYLAND_HEADER_SIZE + sizeof(CurrentId), MSG_DONTWAIT)) {
        Assert(0);
    }
    Context->RegistryId = CurrentId;
    Context->CurrentId = CurrentId;
    wayland_message Message = {WAYLAND_DISPLAY_OBJECT_ID, WAYLAND_WL_DISPLAY_GET_REGISTRY_OPCODE,
                               WAYLAND_HEADER_SIZE + sizeof(Context->CurrentId), 0};

    ScratchEnd(Scratch);
    return Message;
}
