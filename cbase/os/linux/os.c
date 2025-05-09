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

static os_file_handle OsFileOpen(string8 Path, os_access_flags Flags) {
    Temp Scratch = ScratchBegin(0, 0);
    string8 PathCopy = PushString8Copy(Scratch.Arena, Path);

    u32 LinuxFlags = 0;
    if (Flags & Osaccess_Write & Osaccess_Read) {
        LinuxFlags |= O_RDWR;
    } else if (Flags & OsAccess_Write) {
        LinuxFlags |= O_WRONLY;
    } else if (Flags & OsAccess_Read) {
        LinuxFlags |= O_RDONLY;
    }
    if (Flags & OsAccess_Append) {
        LinuxFlags |= O_APPEND;
    }
    if (Flags & (OsAccess_Append | OsAccess_Write)) {
        LinuxFlags |= O_APPEND;
    }
    i32 Fd = open((char *)PathCopy.Str, LinuxFlags, 0755);

    os_file_handle Ret = {0};
    if (Fd >= 0) {
        Ret = Fd;
    }

    ScratchEnd(Scratch);
    return (os_file_handle)(Ret == -1 ? 0 : Ret);
}

static void OsFileClose(os_file_handle File) {
    if (File == 0) return;
    close((i32)File);
}

static b32 OsFileDelete(string8 Path) {
    Temp Scratch = ScratchBegin(0, 0);
    string8 PathCopy = PushString8Copy(Scratch.Arena, Path);
    b32 Ret = 0;
    if (unlink((char *)PathCopy.Str) != -1) Ret = 1;
    ScratchEnd(Scratch);
    return Ret;
}

static file_info *OsFileStat(arena *Arena, string8 Path) {
    // TODO(acol): I need to also extract the name from the path
    string8 PathCopy = PushString8Copy(Arena, Path);
    file_info *FileInfo = ArenaPush(Arena, sizeof(file_info));

    struct stat StatStruct = {0};
    stat((char *)PathCopy.Str, &StatStruct);

    FileInfo->Path = PathCopy;
    FileInfo->Size = (u64)StatStruct.st_size;
    FileInfo->Created = (u64)StatStruct.st_ctim.tv_nsec;
    FileInfo->Modified = (u64)StatStruct.st_mtim.tv_nsec;
    if (StatStruct.st_mode & S_IFDIR) {
        FileInfo->Flags |= FilePropertyFlag_IsFolder;
    }

    return FileInfo;
}

static b32 OsMakeDir(string8 Path) {
    b32 Ret = 0;

    Temp Scratch = ScratchBegin(0, 0);
    string8 PathCopy = PushString8Copy(Scratch.Arena, Path);

    if (mkdir((char *)PathCopy.Str, 0755) != -1) Ret = 1;

    ScratchEnd(Scratch);
    return Ret;
}

static b32 OsDeleteDir(string8 Path) {
    b32 Ret = 0;

    Temp Scratch = ScratchBegin(0, 0);
    string8 PathCopy = PushString8Copy(Scratch.Arena, Path);

    if (rmdir((char *)Path.Str) != -1) Ret = 1;

    ScratchEnd(Scratch);
    return Ret;
}

static u64 OsFileRead(os_file_handle File, v2_u64 ReadWindow, void *Data) {
    if (File == 0) return 0;
    u64 LeftToRead = ReadWindow.Max - ReadWindow.Min;
    u64 Read = 0;
    i32 LastRead = 0;

    while (LeftToRead > 0) {
        LastRead = pread(File, (u8 *)Data + Read, LeftToRead, ReadWindow.Min + Read);
        if (LastRead >= 0) {
            Read += LastRead;
            LeftToRead -= LastRead;
        } else if (errno != EINTR) {
            break;
        }
    }
    return Read;
}

static u64 OsFileWrite(os_file_handle File, v2_u64 WriteWindow, void *Data) {
    if (File == 0) {
        return 0;
    }

    u64 LeftToWrite = WriteWindow.Max - WriteWindow.Min;
    u64 Written = 0;
    i32 LastWrite = 0;

    while (LeftToWrite > 0) {
        LastWrite = pwrite(File, (u8 *)Data + Written, LeftToWrite, WriteWindow.Min + Written);
        if (LastWrite >= 0) {
            LeftToWrite -= LastWrite;
            Written += LastWrite;
        } else if (errno != EINTR) {
            break;
        }
    }
    return Written;
}

static void *OsFileMap(os_file_handle File, os_access_flags Flags, v2_u64 MapWindow) {
    if (File == 0) return (void *)0;

    void *Ret = 0;
    i32 LinuxFlags = 0;

    if (Flags & OsAccess_Read) LinuxFlags |= PROT_READ;
    if (Flags & OsAccess_Write) LinuxFlags |= PROT_WRITE;

    Ret = mmap(0, MapWindow.Max - MapWindow.Min, LinuxFlags, MAP_PRIVATE | MAP_POPULATE, (i32)File,
               MapWindow.Min);

    if (Ret == MAP_FAILED) return (void *)0;
    return Ret;
}

static void OsFileUnmap(void *Ptr, v2_u64 MapWindow) { munmap(Ptr, MapWindow.Max - MapWindow.Min); }

static u64 OsReadTimer(void) {
    struct timeval Time;
    gettimeofday(&Time, 0);
    u64 Ret = 1000000 * (u64)Time.tv_sec + (u64)Time.tv_usec;
    return Ret;
}

static u64 OsTimerFrequency(void) { return 1000000; }
