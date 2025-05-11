static void TctxInitAndEquip(tctx* Tctx) {
    MemoryZeroStruct(Tctx);
    arena** Arenas = Tctx->Arenas;
    for (u32 i = 0; i < ArrayCount(Tctx->Arenas); i++, Arenas++) {
        *Arenas = ArenaAlloc();
    }
    TctxThreadLocal = Tctx;
}

static void TctxRelease(void) {
    for (u32 i = 0; i < ArrayCount(TctxThreadLocal->Arenas); i++) {
        ArenaRelease(TctxThreadLocal->Arenas[i]);
    }
}

static tctx* TctxGetEquiped(void) { return TctxThreadLocal; }

static arena* TctxGetScratch(arena** Conflicts, u64 Count) {
    tctx* Tctx = TctxGetEquiped();

    arena* Ret = 0;

    arena** Arena = Tctx->Arenas;
    for (u32 i = 0; i < ArrayCount(Tctx->Arenas); i++, Arena++) {
        b32 HasConflict = 0;
        for (u32 j = 0; j < Count; j++) {
            if (*Arena == Conflicts[j]) {
                HasConflict = 1;
                break;
            }
        }
        if (!HasConflict) {
            Ret = *Arena;
            break;
        }
    }
    return Ret;
}

static void TctxSetThreadName(string8 Name) {
    tctx* Tctx = TctxGetEquiped();
    u64 Size = ClampTop(Name.Size, sizeof(Tctx->ThreadName));
    MemoryCopy(Tctx->ThreadName, Name.Str, Size);
    Tctx->ThreadNameSize = Size;
}

static string8 TctxGetThreadName(void) {
    tctx* Tctx = TctxGetEquiped();
    return (string8){Tctx->ThreadName, Tctx->ThreadNameSize};
}
