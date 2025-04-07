#ifndef OS_H
#define OS_H

typedef enum os_access_flags {
    OSACCESS_Read = (1 << 0),
    OSACCESS_Write = (1 << 1),
    OSACCESS_Append = (1 << 2),
    OSACCESS_Create = (1 << 3),
    OSACCESS_Execute = (1 << 4),
} os_access_flags;

#if OS_LINUX
static const u64 OS_LARGE_PAGE_SIZE = MB(2);
static const u64 OS_PAGE_SIZE = KB(4);
typedef i32 os_file_handle;
typedef struct file_info {
    // NOTE(acol): Just this for now but will probably extend
    u64 Size = 0;
} file_info;

static void *OsReserve(u64 Size);
static b32 OsCommit(void *Ptr, u64 Size);
static void OsDecommit(void *Ptr, u64 Size);
static void OsRelease(void *Ptr, u64 Size);
static void *OsReserveLarge(u64 Size);
static b32 OsCommitLarge(void *Ptr, u64 Size);
static b32 IsValid(os_file_handle Handle);
static os_file_handle OsFileOpen(string Path, os_access_flags Flags);
static void OsFileClose(os_file_handle File);
static void OsFileDelete(string Path);
static file_info OsFileStat(os_file_handle File);
static file_info OsFileStat(string Path);
static b32 OsMakeDir(string Path);
static b32 OsDeleteDir(string Path);
static u64 OsFileRead(os_file_handle File, void *Data, u64v2 ReadWindow);
static u64 OsFileWrite(os_file_handle File, void *Data, u64v2 WriteWindow);
static void *OsFileMap(os_file_handle File, os_access_flags Flags, u64v2 MapWindow);
static void OsFileUnmap(void *Ptr, u64v2 MapWindow);

static u64 OsReadTimer(void);
static u64 OsTimerFrequency(void);

#elif OS_WINDOWS
    #error Os layer for windows not implemented
#elif OS_MAC
    #error Os layer for mac not implemented
#endif

#endif  // OS_H
