#ifndef BASE_MACROS_H
#define BASE_MACROS_H

// NOTE(acol): makes using macros like normal code more full proof, just a safe guard
#define Statement(S) \
    do {             \
        S            \
    } while (0)

/*  =====================================================================================

                    NOTE(acol): VT codes for terminal

    ===================================================================================== */
// NOTE(acol): Regular text
#define TXT_BLK "\033[0;30m"
#define TXT_RED "\033[0;31m"
#define TXT_GRN "\033[0;32m"
#define TXT_YEL "\033[0;33m"
#define TXT_BLU "\033[0;34m"
#define TXT_MAG "\033[0;35m"
#define TXT_CYN "\033[0;36m"
#define TXT_WHT "\033[0;37m"
// NOTE(acol): Regular bold text
#define TXT_BBLK "\033[1;30m"
#define TXT_BRED "\033[1;31m"
#define TXT_BGRN "\033[1;32m"
#define TXT_BYEL "\033[1;33m"
#define TXT_BBLU "\033[1;34m"
#define TXT_BMAG "\033[1;35m"
#define TXT_BCYN "\033[1;36m"
#define TXT_BWHT "\033[1;37m"
// NOTE(acol): Regular underline text
#define TXT_UBLK "\033[4;30m"
#define TXT_URED "\033[4;31m"
#define TXT_UGRN "\033[4;32m"
#define TXT_UYEL "\033[4;33m"
#define TXT_UBLU "\033[4;34m"
#define TXT_UMAG "\033[4;35m"
#define TXT_UCYN "\033[4;36m"
#define TXT_UWHT "\033[4;37m"
// NOTE(acol): Regular background
#define TXT_BLKB "\033[40m"
#define TXT_REDB "\033[41m"
#define TXT_GRNB "\033[42m"
#define TXT_YELB "\033[43m"
#define TXT_BLUB "\033[44m"
#define TXT_MAGB "\033[45m"
#define TXT_CYNB "\033[46m"
#define TXT_WHTB "\033[47m"
// NOTE(acol): Reset
#define TXT_RST "\033[0m"

/*  =====================================================================================

                    NOTE(acol): OS, Architecture and Compiler detections

    ===================================================================================== */
#if defined(__clang__)
    #define COMPILER_CLANG 1
    #if defined(__gnu_linux__)
        #define OS_LINUX 1
    #elif defined(_WIN32)
        #define OS_WINDOWS 1
    #elif defined(__APPLE__) && defined(__MACH__)
        #define OS_MAC 1
    #else
        #error couldnt detect OS
    #endif
    #if defined(__amd64__)
        #define ARCH_X64 1
    #elif defined(__i386__)
        #define ARCH_X86 1
    #elif defined(__aarch64__)
        #define ARCH_ARM64 1
    #elif defined(__arm__)
        #define ARCH_ARM32 1
    #else
        #error couldnt detect Architecture
    #endif
#elif defined(__GNUC__)
    #define COMPILER_GCC 1
    #if defined(__gnu_linux__)
        #define OS_LINUX 1
    #elif defined(_WIN32)
        #define OS_WINDOWS 1
    #elif defined(__APPLE__) && defined(__MACH__)
        #define OS_MAC 1
    #else
        #error couldnt detect OS
    #endif
    #if defined(__amd64__)
        #define ARCH_X64 1
    #elif defined(__i386__)
        #define ARCH_X86 1
    #elif defined(__aarch64__)
        #define ARCH_ARM64 1
    #elif defined(__arm__)
        #define ARCH_ARM32 1
    #else
        #error couldnt detect Architecture
    #endif
#elif defined(_MSC_VER)
    #define COMPILER_CL 1
    #if defined(_WIN32)
        #define OS_WINDOWS 1
    #else
        #error couldnt detect OS
    #endif
    #if defined(_M_AMD64)
        #define ARCH_x64 1
    #elif defined(_M_I86)
        #define ARCH_x86 1
    #elif defined(_M_ARM)
        #define ARCH_ARM32 1
    #elif defined(_M_ARM64)
        #define ARCH_ARM64 1
    #else
        #error couldnt detect Architecture
    #endif
#else
    #error couldnt detect Compiler
#endif

#if defined(__cplusplus)
    #define LANG_CPP 1
#else
    #define LANG_C 1
#endif

// NOTE(acol): zero filling for if statements
#if !defined(COMPILER_CL)
    #define COMPILER_CL 0
#endif
#if !defined(COMPILER_CLANG)
    #define COMPILER_CLANG 0
#endif
#if !defined(COMPILER_GCC)
    #define COMPILER_GCC 0
#endif
#if !defined(OS_WINDOWS)
    #define OS_WINDOWS 0
#endif
#if !defined(OS_LINUX)
    #define OS_LINUX 0
#endif
#if !defined(OS_MAC)
    #define OS_MAC 0
#endif
#if !defined(ARCH_X64)
    #define ARCH_X64 0
#endif
#if !defined(ARCH_X64)
    #define ARCH_X86 0
#endif
#if !defined(ARCH_X64)
    #define ARCH_ARM64 0
#endif
#if !defined(ARCH_X64)
    #define ARCH_ARM32 0
#endif
#if !defined(LANG_CPP)
    #define LANG_CPP 0
#endif
#if !defined(LANG_C)
    #define LANG_C 0
#endif

#if COMPILER_CLANG || COMPILER_GCC
    #define thread_local __thread
#elif COMPILER_CL
    #define thread_local __declspec(thread)
#endif

#if COMPILER_CLANG || COMPILER_GCC
    #define force_inline __attribute__((always_inline))
#elif COMPILER_MSVC
    #define force_inline __forceinline
#endif

#if LANG_CPP
    #define C_LINKAGE_BEGIN extern "C" {
    #define C_LINKAGE_END }
    #define C_LINKAGE extern "C"
#else
    #define C_LINKAGE_BEGIN
    #define C_LINKAGE_END
    #define C_LINKAGE
#endif

// NOTE(acol): using only asan here but could maybe add os alternatives
#ifdef __SANITIZE_ADDRESS__
    #include <sanitizer/asan_interface.h>
    #define AsanPoison(Address, Size) __asan_poison_memory_region((void *)(Address), (Size))
    #define AsanUnpoison(Address, Size) __asan_unpoison_memory_region((void *)(Address), (Size))

    #if COMPILER_CLANG || COMPILER_GCC
        #define asan_disable __attribute__((no_sanitize_address))
    #else
        #define asan_disable __declspec(no_sanitize_address)
    #endif
#else
    #define AsanPoison(...)
    #define AsanUnpoison(...)
    #define asan_disable
#endif

/*  =====================================================================================

                            NOTE(acol): Timers

    ===================================================================================== */

#define ReadCpuTimer() ((u64)__rdtsc())

/*  =====================================================================================

                            NOTE(acol): Common helper macros

    ===================================================================================== */

// NOTE(acol): Static hints
#if COMPILER_CLANG
    #define Expect(X, Val) __builtin_expect((X), (Val))
#else
    #define Expect(X, Val) (X)
#endif

#define Likely(X) Expect(X, 1)
#define Unlikely(X) Expect(X, 0)

// NOTE(acol): Dodges some warnings by having two levels of expansion
#define Stringify_(S) #S
#define Stringify(S) Stringify_(S)
#define Glue_(A, B) A##B
#define Glue(A, B) Glue_(A, B)

#define Swap(T, A, B) Statement(T t__ = A; A = B; B = t__;)

#define KB(Num) (((u64)(Num)) << 10)
#define MB(Num) (((u64)(Num)) << 20)
#define GB(Num) (((u64)(Num)) << 30)
#define TB(Num) (((u64)(Num)) << 40)
#define Thousand(Num) ((Num) * 1000)
#define Million(Num) ((Num) * 1000000)
#define Billion(Num) ((Num) * 1000000000)

#define AlignPow2(X, AlignTo) (((X) + (AlignTo) - 1) & (~((AlignTo) - 1)))
#define AlignDownPow2(X, AlignTo) ((X) & (~((AlignTo) - 1)))
#define AlignPadPow2(X, AlignTo) ((0 - (X)) & ((AlignTo) - 1))
#define IsPow2(X) ((X) != 0 && ((X) & ((X) - 1)) == 0)
#define IsPow2OrZero(X) ((((X) - 1) & (X)) == 0)

#define ExtractBit(Word, Index) (((Word) >> (Index)) & 1)
#define Compose64Bit(A, B) ((((u64)A) << 32) | ((u64)B))

#define ArrayCount(A) (sizeof(A) / sizeof(A[0]))

#if ARCH_X64
    #define IntFromPointer(P) ((u64)(P))
#elif ARCH_X86
    #define IntFromPointer(P) ((u32)(P))
#else
    #error IntFromPointer not defined for this compiler.
#endif
#define PointerFromInt(I) (void *)((char *)0 + (I))

#if COMPILER_MSVC
    #define AlignOf(T) __alignof(T)
#elif COMPILER_CLANG
    #define AlignOf(T) __alignof(T)
#elif COMPILER_GCC
    #define AlignOf(T) __alignof__(T)
#else
    #error AlignOf not defined for this compiler.
#endif

#define Member_(T, M) (((T *)0)->M)  // NOTE(acol): this is helper macro dont use it :)
#define OffsetOfMember(T, M) IntFromPointer(&Member_(T, M))

#define Min(A, B) (((A) < (B)) ? (A) : (B))
#define Max(A, B) (((A) > (B)) ? (A) : (B))
#define ClampTop(A, X) Min(A, X)
#define ClampBot(X, B) Max(X, B)
#define Clamp(A, X, B) (((X) < (A)) ? (A) : ((X) > (B)) ? (B) : (X))

// NOTE(acol): If you write this the naive way compiler will output something completely nuts, so putting
// it here even tho it's not common to use. For said nuts see: https://godbolt.org/z/91x58asdn
#define PopCount64(A) __popcnt64(A)
#define PopCount32(A) __popcnt32(A)

// TODO(acol): Remove libc dependency
#include <string.h>

#define MemorySet(Dest, Byte, Size) memset((Dest), (Byte), (Size))

#define MemoryCompare(A, B, Size) memcmp((A), (B), (Size))
#define MemoryMatch(A, B, Size) (MemoryCompare((A), (B), (Size)) == 0)
#define MemoryMatchStruct(A, B) MemoryMatch((A), (B), sizeof(*(A)))
#define MemoryMatchArray(A, B, Count) MemoryMatch((A), (B), sizeof(*(A)) * (Count))

#define MemoryZero(Pointer, Size) memset((Pointer), 0, (Size))
#define MemoryZeroStruct(Pointer) MemoryZero((Pointer), sizeof(*(Pointer)))
#define MemoryZeroArray(Pointer, Count) MemoryZero((Pointer), sizeof(*(Pointer)) * (Count))

#define MemoryCopy(Dest, Source, Size) memmove((Dest), (Source), (Size))
#define MemoryCopyStruct(Dest, Source) MemoryCopy((Dest), (Source), Min(sizeof(*(Dest)), sizeof(*(Source))))
#define MemoryCopyArray(Dest, Source, Count) MemoryCopy((Dest), (Source), sizeof(*(Source)) * Count)

/*  =====================================================================================

                                NOTE(acol): Assertions

    ===================================================================================== */

// NOTE(acol): Can change how asserts are handled by defining this somewhere else
// TODO(acol): Remove string stuff from libc
#include <stdio.h>

#if COMPILER_MSVC
    #define Trap() __debugbreak()
#elif COMPILER_GCC
    #define Trap() __builtin_trap()
#elif COMPILER_CLANG
    #define Trap() __builtin_debugtrap()
#else
    #error Unknown trap intrinsic for this compiler.
#endif

#if !defined(AssertBreak)
    #define AssertBreak(C)                                                                    \
        Statement(dprintf(2,                                                                  \
                          TXT_RED "You " TXT_YEL "done " TXT_GRN "did " TXT_BLU "it " TXT_MAG \
                                  "bruv " TXT_UWHT "%s:%d" TXT_RST "\n"                       \
                                  "  Failed condition: " TXT_RED #C TXT_RST "\n",             \
                          __FILE__, __LINE__);                                                \
                  Trap();)
#endif

#define AssertAlways(C) Statement(if (Unlikely(!(C))) { AssertBreak(C); })

#if ASSERT
    #define Assert(C) AssertAlways(C)
#else
    #define Assert(...)
#endif

#define StaticAssert(Condition, Message) _Static_assert((Condition), #Message)

/*  ========================================================================================

      NOTE(acol): Linked list helper macros, to be used with some type of automatic cleanup

    ======================================================================================== */

#define DllPushBack_NP(First, Last, New, Next, Prev)                           \
    ((First) == 0) ? ((First) = (Last) = (New), (New)->Next = (New)->Prev = 0) \
                   : ((New)->Prev = (Last), (Last)->Next = (New), (Last) = (New), (New)->Next = 0)
#define DllPushBack(First, Last, New) DllPushBack_NP(First, Last, New, Next, Prev)
#define DllPushFront(First, Last, New) DllPushBack_NP(Last, First, New, Prev, Next)

#define DllRemoveFirst_N(First, Next) ((First) = (First)->Next, (First)->Prev = 0)
#define DllRemoveFirst(First) DllRemoveFirst_N(First, Next)

#define DllRemoveLast_P(Last, Prev) ((Last) = (Last)->Prev, (Last)->Next = 0)
#define DllRemoveLast(Last) DllRemoveLast_P(Last, Prev)

// NOTE(acol): Remove is a pointer to the node being removed not a value
#define DllRemove_NP(First, Last, Remove, Next, Prev) \
    ((First) == (Remove)  ? (DllRemoveFirst(First))   \
     : (Last) == (Remove) ? (DllRemoveLast(Last))     \
                          : ((Remove)->Next->Prev = (Remove)->Prev, (Remove)->Prev->Next = (Remove)->Next))
#define DllRemove(First, Last, Remove) DllRemove_NP(First, Last, Next, Prev)

#define DllMerge_NP(First, Last, MergeTo, Next, Prev)                                          \
    ((First) == 0           ? 0                                                                \
     : (MergeTo)->Next == 0 ? ((MergeTo)->Next = (First), (First)->Prev = (MergeTo))           \
                            : ((Last)->Next = (MergeTo)->Next, (MergeTo)->Next->Prev = (Last), \
                               (MergeTo)->Next = (First), (First)->Prev = (MergeTo)))
#define DllMerge(First, Last, MergeTo) DllMerge_NP(First, Last, MergeTo, Next, Prev)

// NOTE(acol): Separating stack and queue single linked lists to give the freedom not to store the first
//             element with stacks
#define SllQueuePush_N(First, Last, New, Next) \
    ((First) == 0 ? (First) = (Last) = (New) : ((Last)->Next = (New), (Last) = (New)), (New)->Next = 0)
#define SllQueuePush(First, Last, New) SllQueuePush_N(First, Last, New, Next)

#define SllQueuePushFront_N(First, Last, New, Next) \
    ((First) == 0 ? ((First) = (Last) = (New), (New)->Next = 0) : ((New)->Next = (First), (First) = (New)))
#define SllQueuePushFront(First, Last, New) SllQueuePushFront_N(First, Last, New, Next)

#define SllQueuePop_N(First, Last, Next) ((First) == (Last) ? (First) = (Last) = 0 : (First) = (First)->Next)
#define SllQueuePop(First, Last) SllQueuePop_N(First, Last, Next)

#define SllStackPush_N(Last, New, Next) ((New)->Next = (Last), (Last) = (New))
#define SllStackPush(Last, New) SllStackPush_N(Last, New, Next)

#define SllStackPop_N(Last, Next) ((Last) == 0 ? 0 : (Last) = (Last)->Next)
#define SllStackPop(Last) SllStackPop_N(Last, Next)

/*  =====================================================================================

                                NOTE(acol): Bitmasks

    ===================================================================================== */

#define BITMASK1 0x00000001u
#define BITMASK2 0x00000003u
#define BITMASK3 0x00000007u
#define BITMASK4 0x0000000fu
#define BITMASK5 0x0000001fu
#define BITMASK6 0x0000003fu
#define BITMASK7 0x0000007fu
#define BITMASK8 0x000000ffu
#define BITMASK9 0x000001ffu
#define BITMASK10 0x000003ffu
#define BITMASK11 0x000007ffu
#define BITMASK12 0x00000fffu
#define BITMASK13 0x00001fffu
#define BITMASK14 0x00003fffu
#define BITMASK15 0x00007fffu
#define BITMASK16 0x0000ffffu
#define BITMASK17 0x0001ffffu
#define BITMASK18 0x0003ffffu
#define BITMASK19 0x0007ffffu
#define BITMASK20 0x000fffffu
#define BITMASK21 0x001fffffu
#define BITMASK22 0x003fffffu
#define BITMASK23 0x007fffffu
#define BITMASK24 0x00ffffffu
#define BITMASK25 0x01ffffffu
#define BITMASK26 0x03ffffffu
#define BITMASK27 0x07ffffffu
#define BITMASK28 0x0fffffffu
#define BITMASK29 0x1fffffffu
#define BITMASK30 0x3fffffffu
#define BITMASK31 0x7fffffffu
#define BITMASK32 0xffffffffu
#define BITMASK33 0x00000001ffffffffull
#define BITMASK34 0x00000003ffffffffull
#define BITMASK35 0x00000007ffffffffull
#define BITMASK36 0x0000000fffffffffull
#define BITMASK37 0x0000001fffffffffull
#define BITMASK38 0x0000003fffffffffull
#define BITMASK39 0x0000007fffffffffull
#define BITMASK40 0x000000ffffffffffull
#define BITMASK41 0x000001ffffffffffull
#define BITMASK42 0x000003ffffffffffull
#define BITMASK43 0x000007ffffffffffull
#define BITMASK44 0x00000fffffffffffull
#define BITMASK45 0x00001fffffffffffull
#define BITMASK46 0x00003fffffffffffull
#define BITMASK47 0x00007fffffffffffull
#define BITMASK48 0x0000ffffffffffffull
#define BITMASK49 0x0001ffffffffffffull
#define BITMASK50 0x0003ffffffffffffull
#define BITMASK51 0x0007ffffffffffffull
#define BITMASK52 0x000fffffffffffffull
#define BITMASK53 0x001fffffffffffffull
#define BITMASK54 0x003fffffffffffffull
#define BITMASK55 0x007fffffffffffffull
#define BITMASK56 0x00ffffffffffffffull
#define BITMASK57 0x01ffffffffffffffull
#define BITMASK58 0x03ffffffffffffffull
#define BITMASK59 0x07ffffffffffffffull
#define BITMASK60 0x0fffffffffffffffull
#define BITMASK61 0x1fffffffffffffffull
#define BITMASK62 0x3fffffffffffffffull
#define BITMASK63 0x7fffffffffffffffull
#define BITMASK64 0xffffffffffffffffull

#define BIT1 (1u << 0)
#define BIT2 (1u << 1)
#define BIT3 (1u << 2)
#define BIT4 (1u << 3)
#define BIT5 (1u << 4)
#define BIT6 (1u << 5)
#define BIT7 (1u << 6)
#define BIT8 (1u << 7)
#define BIT9 (1u << 8)
#define BIT10 (1u << 9)
#define BIT11 (1u << 10)
#define BIT12 (1u << 11)
#define BIT13 (1u << 12)
#define BIT14 (1u << 13)
#define BIT15 (1u << 14)
#define BIT16 (1u << 15)
#define BIT17 (1u << 16)
#define BIT18 (1u << 17)
#define BIT19 (1u << 18)
#define BIT20 (1u << 19)
#define BIT21 (1u << 20)
#define BIT22 (1u << 21)
#define BIT23 (1u << 22)
#define BIT24 (1u << 23)
#define BIT25 (1u << 24)
#define BIT26 (1u << 25)
#define BIT27 (1u << 26)
#define BIT28 (1u << 27)
#define BIT29 (1u << 28)
#define BIT30 (1u << 29)
#define BIT31 (1u << 30)
#define BIT32 (1u << 31)
#define BIT33 (1ull << 32)
#define BIT34 (1ull << 33)
#define BIT35 (1ull << 34)
#define BIT36 (1ull << 35)
#define BIT37 (1ull << 36)
#define BIT38 (1ull << 37)
#define BIT39 (1ull << 38)
#define BIT40 (1ull << 39)
#define BIT41 (1ull << 40)
#define BIT42 (1ull << 41)
#define BIT43 (1ull << 42)
#define BIT44 (1ull << 43)
#define BIT45 (1ull << 44)
#define BIT46 (1ull << 45)
#define BIT47 (1ull << 46)
#define BIT48 (1ull << 47)
#define BIT49 (1ull << 48)
#define BIT50 (1ull << 49)
#define BIT51 (1ull << 50)
#define BIT52 (1ull << 51)
#define BIT53 (1ull << 52)
#define BIT54 (1ull << 53)
#define BIT55 (1ull << 54)
#define BIT56 (1ull << 55)
#define BIT57 (1ull << 56)
#define BIT58 (1ull << 57)
#define BIT59 (1ull << 58)
#define BIT60 (1ull << 59)
#define BIT61 (1ull << 60)
#define BIT62 (1ull << 61)
#define BIT63 (1ull << 62)
#define BIT64 (1ull << 63)

#endif  // BASE_MACROS_H
