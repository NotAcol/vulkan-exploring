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
static void ProfileBandwidthStart_(profile_block* restrict Block, const char* restrict Label, u32 AnchorIndex,
                                   u64 ByteCount) {
    Block->ParentIndex = GlobalProfilerParent;
    Block->Label = Label;
    Block->AnchorIndex = AnchorIndex;
    Block->ByteCount = ByteCount;

    profile_anchor* Anchor = GlobalAnchors + AnchorIndex;
    Block->OldTSCElapsedInclusive = Anchor->TSCElapsedInclusive;

    GlobalProfilerParent = AnchorIndex;

    Block->TSCStart = READ_BLOCK_TIMER();
}
static void ProfileBandwidthEnd_(profile_block* Block) {
    u64 Elapsed = READ_BLOCK_TIMER() - Block->TSCStart;
    GlobalProfilerParent = Block->ParentIndex;

    profile_anchor* Parent = GlobalAnchors + Block->ParentIndex;
    profile_anchor* Anchor = GlobalAnchors + Block->AnchorIndex;

    Anchor->TSCElapsedInclusive = Block->OldTSCElapsedInclusive + Elapsed;
    Parent->TSCElapsedExclusive -= Elapsed;
    Anchor->TSCElapsedExclusive += Elapsed;
    Anchor->ProcessedBytes += Block->ByteCount;
    ++Anchor->HitCount;

    Anchor->Label = Block->Label;
}

static void PrintTimeElapsed(u64 TotalTSCElapsed, u64 TimerFrequency, profile_anchor* Anchor) {
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
        profile_anchor* Anchor = GlobalAnchors + i;
        if (Anchor->TSCElapsedInclusive) {
            PrintTimeElapsed(TotalTSCElapsed, TimerFrequency, Anchor);
        }
    }
}

#endif

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
