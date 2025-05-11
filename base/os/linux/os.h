#ifndef OS_H
#define OS_H

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>

typedef enum os_access_flags {
    OsAccess_Read = (1 << 0),
    OsAccess_Write = (1 << 1),
    OsAccess_Append = (1 << 2),
    OsAccess_Execute = (1 << 3),
} os_access_flags;

static const u64 OS_LARGE_PAGE_SIZE = MB(2);
static const u64 OS_PAGE_SIZE = KB(4);

typedef u32 os_file_handle;

typedef u32 file_property_flags;
enum {
    FilePropertyFlag_IsDirectory = (1 << 0),
    FilePropertyFlag_IsSymlink = (1 << 1),
};

typedef struct file_info {
    string8 Name;
    u64 Size;
    u64 Modified;
    u64 Created;
    file_property_flags Flags;
} file_info;

static void *OsReserve(u64 Size);
static b32 OsCommit(void *Ptr, u64 Size);
static void OsDecommit(void *Ptr, u64 Size);
static void OsRelease(void *Ptr, u64 Size);
static void *OsReserveLarge(u64 Size);
static b32 OsCommitLarge(void *Ptr, u64 Size);

static os_file_handle OsSharedMemoryAlloc(string8 Name, u64 Size);
static os_file_handle OsSharedMemoryOpen(string8 Name);
static void OsSharedMemoryClose(os_file_handle Handle);
static void *OsSharedMemoryMap(os_file_handle Handle, v2_u64 MapWindow);
static void OsSharedMemoryUnmap(os_file_handle Handle, void *Ptr, v2_u64 MapWindow);

static void *RingBufferAlloc(string8 Name, u64 Size);
static void RingBufferRelease(void *RingBuffer, u64 RingSize);

static os_file_handle OsFileOpen(string8 Path, os_access_flags Flags);
static void OsFileClose(os_file_handle File);
static b32 OsFileDelete(string8 Path);
// static file_info OsFileInfoFromHandle(os_file_handle File);
static file_info *OsFileStat(arena *Arena, string8 Path);
static u64 OsFileRead(os_file_handle File, v2_u64 ReadWindow, void *Data);
static u64 OsFileWrite(os_file_handle File, v2_u64 WriteWindow, void *Data);
static void *OsFileMap(os_file_handle File, os_access_flags Flags, v2_u64 MapWindow);
static void OsFileUnmap(void *Ptr, v2_u64 MapWindow);

static b32 OsMakeDir(string8 Path);
static b32 OsDeleteDir(string8 Path);

static u64 OsReadTimer(void);
static u64 OsTimerFrequency(void);

#endif  // OS_H
