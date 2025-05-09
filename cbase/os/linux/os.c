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
    void *Temp = mmap(0, Size, PROT_NONE,
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
