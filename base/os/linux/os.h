#ifndef OS_H
#define OS_H

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dlfcn.h>

typedef enum os_access_flags {
    OsAccess_Read    = (1 << 0),
    OsAccess_Write   = (1 << 1),
    OsAccess_Append  = (1 << 2),
    OsAccess_Execute = (1 << 3),
} os_access_flags;

static const u64 OS_LARGE_PAGE_SIZE = MB(2);
static const u64 OS_PAGE_SIZE       = KB(4);

typedef u64 os_handle;

typedef u32 file_property_flags;
enum {
    FilePropertyFlag_IsDirectory = (1 << 0),
    FilePropertyFlag_IsSymlink   = (1 << 1),
};

typedef struct file_info {
    string8 Name;
    u64 Size;
    u64 Modified;
    u64 Created;
    file_property_flags Flags;
} file_info;

typedef struct ring_buffer {
    u8 *Data;
    u64 RingSize;
    u64 Read;
    u64 Written;
} ring_buffer;

static u64 OsReadTimer(void);
static u64 OsTimerFrequency(void);

static void *OsReserve(u64 Size);
static b32 OsCommit(void *Ptr, u64 Size);
static void OsDecommit(void *Ptr, u64 Size);
static void OsRelease(void *Ptr, u64 Size);
static void *OsReserveLarge(u64 Size);
static b32 OsCommitLarge(void *Ptr, u64 Size);

static os_handle OsSharedMemoryAlloc(string8 Name, u64 Size);
static void OsSharedMemoryDelete(string8 Name);
static os_handle OsSharedMemoryOpen(string8 Name);
static void OsSharedMemoryClose(os_handle Handle);
static void *OsSharedMemoryMap(os_handle Handle, v2_u64 MapWindow);
static void OsSharedMemoryUnmap(os_handle Handle, void *Ptr, v2_u64 MapWindow);

static os_handle OsFileOpen(string8 Path, os_access_flags Flags);
static void OsFileClose(os_handle File);
static b32 OsFileDelete(string8 Path);
// static file_info OsFileInfoFromHandle(os_handle File);
static file_info *OsFileStat(arena *Arena, string8 Path);
static u64 OsFileRead(os_handle File, v2_u64 ReadWindow, void *Data);
static u64 OsFileWrite(os_handle File, v2_u64 WriteWindow, void *Data);
static void *OsFileMap(os_handle File, os_access_flags Flags, v2_u64 MapWindow);
static void OsFileUnmap(void *Ptr, v2_u64 MapWindow);

static b32 OsMakeDir(string8 Path);
static b32 OsDeleteDir(string8 Path);

static os_handle OsLibraryOpen(string8 Path);
static void *OsLibraryLoadSymbol(os_handle Lib, string8 Symbol);
static void OsLibraryClose(os_handle Lib);

// NOTE(acol):  Keeping this shit here cause it's a little hacky and it uses os stuff heavily in linux and
//              windows
static ring_buffer RingBufferAlloc(u64 Size);
static void RingBufferRelease(ring_buffer *RingBuffer);
static void *RingBufferWritePtr(ring_buffer *RingBuffer);
static void RingBufferWriteEnd(ring_buffer *RingBuffer, u64 Size);
static void *RingBufferWrite(ring_buffer *RingBuffer, void *From, u64 Size);
static void *RingBufferReadPtr(ring_buffer *RingBuffer);
static void RingBufferReadEnd(ring_buffer *RingBuffer, u64 Size);
static void *RingBufferRead(ring_buffer *RingBuffer, void *To, u64 Size);

#endif  // OS_H
