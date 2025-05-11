#ifndef INTRINSICS_H
#define INTRINSICS_H

#if INTRINSICS_MICROSOFT
    #include <intrin.h>
#elif ARCH_X86 || ARCH_X64
    #include <x86intrin.h>
#endif

#endif  // INTRINSICS_H
