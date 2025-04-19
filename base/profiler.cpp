static u64 ReadCpuTimer(void) { return __rdtsc(); }

static u64 EstimateBlockTimerFreq(void) {
    u64 MsToWait = 100;

    u64 OsEnd = 0;
    u64 OsElapsed = 0;
    u64 OsFreq = OsTimerFrequency();
    u64 OsStart = OsReadTimer();
    u64 BlockStart = READ_BLOCK_TIMER();

    while (OsElapsed < ((MsToWait * OsFreq) / 1000)) {
        OsEnd = OsReadTimer();
        OsElapsed = OsEnd - OsStart;
    }

    u64 BlockElapsed = READ_BLOCK_TIMER() - BlockStart;
    return BlockElapsed * OsFreq / OsElapsed;
}

#if PROFILE

profile_block::profile_block(const char *Label_, u32 AnchorIndex_, u64 ByteCount_) {
    AnchorIndex = AnchorIndex_;
    ParentIndex = GlobalProfilerParent;
    GlobalProfilerParent = AnchorIndex;
    Label = Label_;
    ByteCount = ByteCount_;

    profile_anchor *Anchor = GlobalAnchors + AnchorIndex;
    OldTSCElapsedInclusive = Anchor->TSCElapsedInclusive;

    TSCStart = READ_BLOCK_TIMER();
}
profile_block::~profile_block(void) {
    u64 Elapsed = READ_BLOCK_TIMER() - TSCStart;
    GlobalProfilerParent = ParentIndex;

    profile_anchor *Anchor = GlobalAnchors + AnchorIndex;
    profile_anchor *Parena = GlobalAnchors + ParentIndex;

    // NOTE(acol): overwriting the deeper recursion calls
    Anchor->TSCElapsedInclusive = OldTSCElapsedInclusive + Elapsed;

    Anchor->TSCElapsedExclusive += Elapsed;
    Parena->TSCElapsedExclusive -= Elapsed;
    Anchor->ProcessedBytes += ByteCount;

    Anchor->Label = Label;
    ++Anchor->HitCount;
}

static void PrintTimeElapsed(u64 TotalTSCElapsed, u64 TimerFrequency, profile_anchor *Anchor) {
    f64 PercentTime = 100.0 * ((f64)Anchor->TSCElapsedExclusive / (f64)TotalTSCElapsed);

    printf("  " TXT_BLU "%s[" TXT_WHT "%llu" TXT_BLU "]: " TXT_WHT "%llu (" TXT_RED "%.2f%%" TXT_WHT,
           Anchor->Label, Anchor->HitCount, Anchor->TSCElapsedExclusive, PercentTime);

    if (Anchor->TSCElapsedInclusive != Anchor->TSCElapsedExclusive) {
        f64 PercentTimeInclusive = 100.0 * (f64)Anchor->TSCElapsedInclusive / (f64)TotalTSCElapsed;
        printf(", " TXT_RED "%.2f%%" TXT_WHT " w/children", PercentTimeInclusive);
    }
    printf(")");
    if (Anchor->ProcessedBytes) {
        f64 Seconds = (f64)Anchor->TSCElapsedInclusive / (f64)TimerFrequency;
        f64 BytesPerSecond = (f64)Anchor->ProcessedBytes / Seconds;

        printf(TXT_BLU " | " TXT_WHT "%.3fMB at " TXT_RED "%.2f" TXT_WHT "GiB/s",
               (f64)Anchor->ProcessedBytes / (f64)MB(1), BytesPerSecond / (f64)GB(1));
    }
    printf(TXT_RST "\n");
}

static void PrintAnchorData(u64 TotalTSCElapsed, u64 TimerFrequency) {
    for (u32 i = 0; i < ArrayCount(GlobalAnchors); i++) {
        profile_anchor *Anchor = GlobalAnchors + i;
        if (Anchor->TSCElapsedInclusive) {
            PrintTimeElapsed(TotalTSCElapsed, TimerFrequency, Anchor);
        }
    }
}

#endif  // PROFILE

static void BeginProfile(void) { GlobalProfiler.StartTSC = READ_BLOCK_TIMER(); }

static void EndAndPrintProfile(void) {
    GlobalProfiler.EndTSC = READ_BLOCK_TIMER();
    u64 TimerFrequency = EstimateBlockTimerFreq();
    u64 TotalTSCElapsed = GlobalProfiler.EndTSC - GlobalProfiler.StartTSC;

    if (TimerFrequency) {
        printf("\n" TXT_BLU "Total time" TXT_WHT " %0.4fms (" TXT_BLU "Timer frequency" TXT_WHT
               " %llu)" TXT_RST "\n",
               1000.0 * (f64)TotalTSCElapsed / (f64)TimerFrequency, TimerFrequency);
    }
    PrintAnchorData(TotalTSCElapsed, TimerFrequency);
}
