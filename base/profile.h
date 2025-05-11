#ifndef PROFILE_H
#define PROFILE_H
// TODO(acol): this will have to be changed to work with multithreading but for now it's whatever

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
    const char* Label;
} profile_anchor;

static profile_anchor GlobalAnchors[4096];
static u32 GlobalProfilerParent;

typedef struct profile_block {
    // NOTE(acol): To deal with recursion we read the elapsed time till now and at the end write it back +
    //             elapsed in this block, overriding the fuckery inner blocks introduce
    const char* Label;
    u64 OldTSCElapsedInclusive;
    u64 TSCStart;
    u64 ParentIndex;
    u64 AnchorIndex;
    u64 ByteCount;
} profile_block;

static void ProfileBandwidthStart_(profile_block* restrict Block, const char* restrict Label, u32 AnchorIndex,
                                   u64 ByteCount);
static void ProfileBandwidthEnd_(profile_block* Block);

static void PrintTimeElapsed(u64 TotalTSCElapsed, u64 TimerFrequency, profile_anchor* Anchor);
static void PrintAnchorData(u64 TotalTSCElapsed, u64 TimerFrequency);

    #define ProfileBandwidthStart(Name, Bytes) \
        profile_block Name##123;               \
        ProfileBandwidthStart_(&Name##123, #Name, __COUNTER__ + 1, Bytes)

    #define ProfileBandwidthEnd(Name) ProfileBandwidthEnd_(&Name##123)

    #define ProfileFunctionStart()   \
        profile_block __func__##123; \
        ProfileBandwidthStart_(&__func__##123, __func__, __COUNTER__ + 1, 0)

    #define ProfileFunctionEnd() ProfileBandwidthEnd_(&__func__##123)

    #define PROFILER_END_OF_COMPILATION_UNIT \
        StaticAssert(__COUNTER__ < ArrayCount(GlobalAnchors), Too many profile anchors)

#else

    #define ProfileFunctionStart(...)
    #define ProfileFunctionEnd(...)
    #define ProfileBandwidthStart(...)
    #define ProfileBandwidthEnd(...)
    #define PrintAnchorData(...)
    #define PROFILER_END_OF_COMPILATION_UNIT

#endif

#define ProfileBlockStart(Name) ProfileBandwidthStart(Name, 0)
#define ProfileBlockEnd(Name) ProfileBandwidthEnd(Name)

typedef struct profiler {
    u64 StartTSC;
    u64 EndTSC;
} profiler;
static profiler GlobalProfiler;

static void BeginProfile(void);
static void EndAndPrintProfile(void);

#endif  // PROFILE_H
