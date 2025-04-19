#if OS_LINUX
    #include <fcntl.h>
    #include <unistd.h>
    #include <errno.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <sys/time.h>

static void *OsReserve(u64 Size) {
    void *Temp = mmap(0, Size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    return Temp == MAP_FAILED ? (void *)0 : Temp;
}

static b32 OsCommit(void *Ptr, u64 Size) {
    if (mprotect(Ptr, Size, PROT_READ | PROT_WRITE) == -1) return 0;
    if (madvise(Ptr, Size, MADV_POPULATE_WRITE) == -1) return 0;
    return 1;
}

static void OsDecommit(void *Ptr, u64 Size) {
    mprotect(Ptr, Size, PROT_NONE);
    madvise(Ptr, Size, MADV_DONTNEED);
}

static void OsRelease(void *Ptr, u64 Size) { munmap(Ptr, Size); }

static void *OsReserveLarge(u64 Size) {
    void *Temp =
        mmap(0, Size, PROT_NONE,
             MAP_HUGETLB | MAP_HUGE_2MB | MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    return Temp == MAP_FAILED ? (void *)0 : Temp;
}

static b32 OsCommitLarge(void *Ptr, u64 Size) {
    if (mprotect(Ptr, Size, PROT_READ | PROT_WRITE) == -1) return 0;
    if (madvise(Ptr, Size, MADV_POPULATE_WRITE) == -1) return 0;
    return 1;
}

static b32 IsValid(os_file_handle Handle) { return Handle > 0; }

static os_file_handle OsFileOpen(string Path, os_access_flags Flags) {
    u32 LinuxFlags = 0;
    if (Flags & OSACCESS_Write & OSACCESS_Read) {
        LinuxFlags |= O_RDWR;
    } else if (Flags & OSACCESS_Read) {
        LinuxFlags |= O_RDONLY;
    } else if (Flags & OSACCESS_Write) {
        LinuxFlags |= O_WRONLY;
    }
    if (Flags & OSACCESS_Append) {
        LinuxFlags |= O_APPEND;
    }
    if (Flags & OSACCESS_Append) {
        LinuxFlags |= O_APPEND;
    }
    if (Flags & OSACCESS_Create) {
        LinuxFlags |= O_CREAT;
    }
    i32 Ret = open((char *)Path.Str, LinuxFlags, 0755);
    return (os_file_handle)(Ret == -1 ? 0 : Ret);
}

static void OsFileClose(os_file_handle File) {
    if (IsValid(File)) {
        close((i32)File);
    }
}

static void OsFileDelete(string Path) { unlink((char *)Path.Str); }

static file_info OsFileStat(os_file_handle File) {
    struct stat StatStruct = {};
    fstat(File, &StatStruct);
    file_info FileInfo = {};
    FileInfo.Size = (u64)StatStruct.st_size;
    FileInfo.LastModified = (u64)StatStruct.st_ctim.tv_nsec;
    return FileInfo;
}

static file_info OsFileStat(string Path) {
    struct stat StatStruct = {};
    stat((char *)Path.Str, &StatStruct);
    file_info FileInfo = {};
    FileInfo.Size = (u64)StatStruct.st_size;
    FileInfo.LastModified = (u64)StatStruct.st_ctim.tv_nsec;
    return FileInfo;
}

static b32 OsMakeDir(string Path) {
    if (mkdir((char *)Path.Str, 0755) != -1) return 1;
    return 0;
}
static b32 OsDeleteDir(string Path) {
    if (rmdir((char *)Path.Str) != -1) return 1;
    return 0;
}

static u64 OsFileRead(os_file_handle File, void *Data, u64v2 ReadWindow) {
    u64 Read = 0;
    if (!IsValid(File)) {
        return Read;
    }
    if (lseek(File, ReadWindow.Min, SEEK_SET) == -1) return 0;

    u64 LeftToRead = ReadWindow.Max - ReadWindow.Min;
    i32 LastRead = 0;

    while (LeftToRead != 0) {
        LastRead = read(File, (u8 *)Data + Read, LeftToRead);
        if (LastRead >= 0) {
            Read += LastRead;
            LeftToRead -= LastRead;
        } else if (errno != EINTR) {
            break;
        }
    }
    return Read;
}

static u64 OsFileWrite(os_file_handle File, void *Data, u64v2 WriteWindow) {
    u64 Written = 0;
    if (!IsValid(File)) {
        return Written;
    }
    if (lseek(File, WriteWindow.Min, SEEK_SET) == -1) return 0;

    u64 LeftToWrite = WriteWindow.Max - WriteWindow.Min;
    i32 LastWrite = 0;

    while (LeftToWrite != 0) {
        LastWrite = write(File, (u8 *)Data + Written, LeftToWrite);
        if (LastWrite > 0) {
            LeftToWrite -= LastWrite;
            Written += LastWrite;
        } else if (errno != EINTR) {
            break;
        }
    }
    return Written;
}

static void *OsFileMap(os_file_handle File, os_access_flags Flags, u64v2 MapWindow) {
    void *Ret = 0;
    u64 LinuxFlags = 0;
    if (!IsValid(File)) return (void *)0;

    if (Flags & OSACCESS_Read) LinuxFlags |= PROT_READ;
    if (Flags & OSACCESS_Write) LinuxFlags |= PROT_WRITE;

    Ret = mmap(0, MapWindow.Max - MapWindow.Min, LinuxFlags, MAP_PRIVATE | MAP_POPULATE, File,
               MapWindow.Min);

    if (Ret == MAP_FAILED) return (void *)0;
    return Ret;
}

static void OsFileUnmap(void *Ptr, u64v2 MapWindow) { munmap(Ptr, MapWindow.Max - MapWindow.Min); }

static u64 OsReadTimer(void) {
    struct timeval Time;
    gettimeofday(&Time, 0);
    u64 Ret = 1000000 * (u64)Time.tv_sec + (u64)Time.tv_usec;
    return Ret;
}

static u64 OsTimerFrequency(void) { return 1000000; }

#elif OS_WINDOWS
    #error Os layer for windows not implemented
#elif OS_MAC
    #error Os layer for mac not implemented
#endif
