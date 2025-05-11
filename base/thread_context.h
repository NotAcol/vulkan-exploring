#ifndef THREAD_CONTEXT_H
#define THREAD_CONTEXT_H
// TODO(acol): doing some very basic stuff here just to get the rest up and running but threding needs hella
// work still

// NOTE(acol): these need a "thread_local tctx *TctxThreadLocal" declared somewhere to function

typedef struct tctx {
    arena* Arenas[2];
    u8 ThreadName[32];
    u64 ThreadNameSize;
} tctx;

typedef void thread_function(void*);

static void TctxInitAndEquip(tctx* Tctx);
static void TctxRelease(void);
static tctx* TctxGetEquiped(void);

static arena* TctxGetScratch(arena** Conflicts, u64 Count);

static void TctxSetThreadName(string8 Name);
static string8 TctxGetThreadName(void);

#define ScratchBegin(Conflicts, Count) TempBegin(TctxGetScratch((Conflicts), (Count)))
#define ScratchEnd(Scratch) TempEnd((Scratch))
#endif  // THREAD_CONTEXT_H
