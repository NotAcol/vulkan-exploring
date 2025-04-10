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
    #define thread_static __thread
#elif COMPILER_CL
    #define thread_static __declspec(thread)
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
        #define AsanDisable __attribute__((no_sanitize_address))
    #else
        #define AsanDisable __declspec(no_sanitize_address)
    #endif
#else
    #define AsanPoison(...)
    #define AsanUnpoison(...)
    #define AsanDisable
#endif

/*  =====================================================================================

                            NOTE(acol): Common helper macros

    ===================================================================================== */

// NOTE(acol): Static hints
#if COMPILER_CL
    #define Likely(x) (x)
    #define Unlikely(x) (x)
#else
    #define Likely(x) __builtin_expect(!!(x), 1)
    #define Unlikely(x) __builtin_expect(!!(x), 0)
#endif

// NOTE(acol): Dodges some warnings by having two levels of expansion
#define ToString_(S) #S
#define ToString(S) ToString_(S)
#define Glue_(A, B) A##B
#define Glue(A, B) Glue_(A, B)

#define KB(Num) (((u64)(Num)) << 10)
#define MB(Num) (((u64)(Num)) << 20)
#define GB(Num) (((u64)(Num)) << 30)
#define TB(Num) (((u64)(Num)) << 40)
#define Thousand(Num) ((Num) * 1000)
#define Million(Num) ((Num) * 1000000)
#define Billion(Num) ((Num) * 1000000000)

#define AlignPow2(X, AlignTo) (((X) + (AlignTo) - 1) & (~((AlignTo) - 1)))

#define ArrayCount(A) (sizeof(A) / sizeof(A[0]))

#define IntFromPointer(P) (u64)((char *)P - (char *)0)
#define PointerFromInt(I) (void *)((char *)0 + (I))

#define Member_(T, M) (((T *)0)->M)  // NOTE(acol): this is helper macro dont use it :)
#define OffsetOfMember(T, M) IntFromPointer(&Member_(T, M))
#define SizeOfMember(T, M) sizeof(Member_(T, M))

#define Min(A, B) (((A) < (B)) ? (A) : (B))
#define Max(A, B) (((A) > (B)) ? (A) : (B))
#define ClampTop(A, X) Min(A, X)
#define ClampBot(X, B) Max(X, B)
#define Clamp(A, X, B) (((X) < (A)) ? (A) : ((X) > (B)) ? (B) : (X))

// NOTE(acol): If you write this the naive way compiler will output something completely nuts, so putting
// it here even tho it's not common to use. For said nuts see: https://godbolt.org/z/91x58asdn
#if COMPILER_CL
    #define PopCount(A) __popcnt64(A)
#else
    #define PopCount(A) __builtin_popcount(A)
#endif

// TODO(acol): Remove libc dependency
#include <string.h>
#define MemoryZero(Pointer, Size) memset((Pointer), 0, (Size))
#define MemoryZeroStruct(Pointer) MemoryZero((Pointer), sizeof(*(Pointer)))
#define MemoryZeroArray(Pointer) MemoryZero((Pointer), sizeof(Pointer))
#define MemoryZeroTyped(Pointer, Count) MemoryZero((Pointer), sizeof(*(Pointer)) * (Count))

#define MemoryMatch(A, B, Size) (memcmp((A), (B), (Size)) == 0)
#define MemoryCompare(A, B, Size) memcmp((A), (B), (Size))

#define MemoryCopy(Dest, Source, Size) memmove((Dest), (Source), (Size))
#define MemoryCopyStruct(Dest, Source) MemoryCopy((Dest), (Source), Min(sizeof(*(Dest)), sizeof(*(Source))))
#define MemoryCopyArray(Dest, Source) MemoryCopy((Dest), (Source), Min(sizeof((Dest)), sizeof((Source))))
#define MemoryCopyTyped(Dest, Source, Count) \
    MemoryCopy((Dest), (Source), Min(sizeof(*(Dest)), sizeof(*(Source))) * Count)

/*  =====================================================================================

                                NOTE(acol): Assertions

    ===================================================================================== */

// NOTE(acol): Can change how asserts are handled by defining this somewhere else
// TODO(acol): Remove string stuff from libc
#include <stdio.h>
#if !defined(AssertBreak)
    #if COMPILER_GCC  // NOTE(acol): gcc sucks such major ass with this...
        #define AssertBreak(C)                                                                    \
            Statement(dprintf(2,                                                                  \
                              TXT_RED "You " TXT_YEL "done " TXT_GRN "did " TXT_BLU "it " TXT_MAG \
                                      "bruv " TXT_UWHT "%s:%d" TXT_RST "\n"                       \
                                      "  Failed condition: " TXT_RED #C TXT_RST "\n",             \
                              __FILE__, __LINE__);                                                \
                      __builtin_trap();)
    #elif COMPILER_CL
        #define AssertBreak(C)                                                                    \
            Statement(dprintf(2,                                                                  \
                              TXT_RED "You " TXT_YEL "done " TXT_GRN "did " TXT_BLU "it " TXT_MAG \
                                      "bruv " TXT_UWHT "%s:%d" TXT_RST "\n"                       \
                                      "  Failed condition: " TXT_RED #C TXT_RST "\n",             \
                              __FILE__, __LINE__);                                                \
                      __debugbreak();)
    #else
        #define AssertBreak(C)                                                                    \
            Statement(dprintf(2,                                                                  \
                              TXT_RED "You " TXT_YEL "done " TXT_GRN "did " TXT_BLU "it " TXT_MAG \
                                      "bruv " TXT_UWHT "%s:%d" TXT_RST "\n"                       \
                                      "  Failed condition: " TXT_RED #C TXT_RST "\n",             \
                              __FILE__, __LINE__);                                                \
                      __builtin_debugtrap();)
    #endif
#endif

#if defined(ENABLE_ASSERT)
    #define Assert(C) Statement(if (Unlikely(!(C))) { AssertBreak(C); })
#else
    #define Assert(...)
#endif

#define StaticAssert(Condition, Message) static_assert((Condition), #Message)

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

#endif  // BASE_MACROS_H
