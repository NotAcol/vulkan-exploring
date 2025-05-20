#include "base_include.h"

thread_local tctx *TctxThreadLocal;
#include "base_include.c"

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>

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
  u32 XdgWmBaseId;
  u32 WlCompositorId;

  u32 Width;
  u32 Height;

  ring_buffer RingBuffer;

  // TODO
  u32 wl_shm;
  u32 wl_shm_pool;
  u32 wl_buffer;
  u32 xdg_surface;
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

  AlwaysAssert(Header.Size == send(Fd, Buff, Header.Size, MSG_DONTWAIT));

  Context->RegistryId = CurrentId;
  Context->CurrentId = CurrentId;
  ScratchEnd(Scratch);
}

static u32 WaylandRegistryBind(wayland_context *Context, u32 Name, u32 Version, u32 InterfaceLen) {
  temp_arena Scratch = ScratchBegin(0, 0);

  ring_buffer RingBuffer = Context->RingBuffer;
  u32 CurrentId = Context->CurrentId;
  i32 Fd = Context->SocketFd;
  wayland_header MessageHeader = {.ResourceId = Context->RegistryId,
                                  .Opcode = WAYLAND_WL_REGISTRY_BIND_OPCODE,
                                  .Size = sizeof(wayland_header) + sizeof(Name) + AlignPow2(InterfaceLen, 4) +
                                          sizeof(Version) + sizeof(CurrentId)};
  ++CurrentId;
  Assert(AlignPow2(MessageHeader.Size) == MessageHeader.Size);

  u8 *Buff = (u8 *)ArenaPush(Scratch.Arena, MessageHeader.Size);
  u8 *Temp = Buff;

  *(wayland_header *)Buff = MessageHeader;
  Buff += sizeof(wayland_header);

  *(u32 *)Buff = Name;
  Buff += sizeof(Name);

  // MemoryCopy(Buff, RingBuffer.Data + (RingBuffer.Read & (RingBuffer.RingSize - 1)), InterfaceLen);
  RingBufferRead(RingBuffer, Buff, InterfaceLen);
  Buff += InterfaceLen;

  *(u32 *)Buff = Version;
  Buff += sizeof(Version);

  *(u32 *)Buff = CurrentId;
  Buff += sizeof(CurrentId);

  AlwaysAssert(send(Fd, Temp, MessageHeader.Size, MSG_DONTWAIT) == MessageHeader.Size);

  Context->CurrentId = CurrentId;
  Context->RingBuffer = RingBuffer;
  ScratchEnd(Scratch);
  return CurrentId;
}

i32 main(void) {
  tctx Tctx;
  TctxInitAndEquip(&Tctx);

  AlwaysAssert(sizeof(wayland_header) == WAYLAND_HEADER_SIZE);
  wayland_context Context;

  Context.RingBuffer = RingBufferAlloc(KB(4));

  WaylandDisplayConnect(&Context);
  WaylandGetRegistry(&Context);

  RingBufferRelease(Context.RingBuffer);
  TctxRelease();
  printf("Made it\n");
  return 0;
}
