#ifndef PROFILER_H
#define PROFILER_H

static u64 ReadCpuTimer(void);

#ifndef READ_BLOCK_TIMER
    #define READ_BLOCK_TIMER ReadCpuTimer
#endif

static u64 EstimateBlockTimerFreq(void);

#ifndef PROFILE
    #define PROFILE 0
#endif

#if PROFILE

typedef struct profile_anchor {
    u64 TSCElapsedInclusive;
    u64 TSCElapsedExclusive;
    u64 HitCount;

    u64 ProcessedBytes;
    const char *Label;
} profile_anchor;

static profile_anchor GlobalAnchors[4096];
static u32 GlobalProfilerParent;

typedef struct profile_block {
    profile_block(const char *Label_, u32 AnchorIndex_, u64 ByteCount_);
    ~profile_block(void);
    const char *Label;
    u64 OldTSCElapsedInclusive;
    u64 TSCStart;
    u64 ParentIndex;
    u64 AnchorIndex;
    u64 ByteCount;
} profile_block;

    #define TimeBandwidth(Name, ByteCount) \
        profile_block Glue(Block, __LINE__)(Name, __COUNTER__ + 1, ByteCount)
    #define PROFILER_END_OF_COMPILATION_UNIT \
        StaticAssert(__COUNTER__ < ArrayCount(GlobalAnchors), Too many profile anchors)

static void PrintTimeElapsed(u64 TotalTSCElapsed, u64 TimerFrequency, profile_anchor *Anchor);
static void PrintAnchorData(u64 TotalTSCElapsed, u64 TimerFrequency);
#else

    #define TimeBandwidth(...)
    #define PrintAnchorData(...)
    #define PROFILER_END_OF_COMPILATION_UNIT
#endif  // PROFILE

#define ProfileBlock(Name) TimeBandwidth(Name, 0)
#define ProfileFunction() ProfileBlock(__func__)

typedef struct profiler {
    u64 StartTSC;
    u64 EndTSC;
} profiler;
static profiler GlobalProfiler;

static void BeginProfile(void);
static void EndAndPrintProfile(void);
#endif  // PROFILER_H
