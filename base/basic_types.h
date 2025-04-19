#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

/*  =====================================================================================

                        NOTE(acol): Basic types and some useful constants

    ===================================================================================== */

typedef double f64;
typedef float f32;

typedef signed char i8;
typedef signed short int i16;
typedef signed int i32;
typedef signed long long int i64;

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

typedef u8 b8;
typedef u16 b16;
typedef u32 b32;
typedef u64 b64;

static const i8 MinI8 = (i8)0x80;
static const i16 MinI16 = (i16)0x8000;
static const i32 MinI32 = (i32)0x80000000;
static const i64 MinI64 = (i64)0x8000000000000000llu;

static const i8 MaxI8 = (i8)0x7f;
static const i16 MaxI16 = (i16)0x7fff;
static const i32 MaxI32 = (i32)0x7fffffff;
static const i64 MaxI64 = (i64)0x7fffffffffffffffllu;

static const u8 MaxU8 = (i8)0xff;
static const u16 MaxU16 = (i16)0xffff;
static const u32 MaxU32 = (i32)0xffffffff;
static const u64 MaxU64 = (i64)0xffffffffffffffffllu;

static const f32 MachineEpsilonF32 = (f32)1.1920929e-7f;
static const f32 PiF32 = (f32)3.14159265359f;
static const f32 EF32 = (f32)2.71828182846f;

static const f64 MachineEpsilonF64 = (f64)2.220446e-16;
static const f64 PiF64 = (f64)3.14159265359;
static const f64 EF64 = (f32)2.71828182846;

// NOTE(acol): Compound types
typedef union v2 {
    struct {
        f32 X;
        f32 Y;
    };
    f32 V[2];
} v2;

typedef union v3 {
    struct {
        f32 X;
        f32 Y;
        f32 Z;
    };
    f32 V[3];
} v3;

typedef union v4 {
    struct {
        f32 X;
        f32 Y;
        f32 Z;
        f32 W;
    };
    f32 V[4];
} v4;

// NOTE(acol): intiger vector 2
typedef union iv2 {
    struct {
        i32 X;
        i32 Y;
    };
    struct {
        i32 Min;
        i32 Max;
    };
    i32 V[2];
} iv2;

typedef union u64v2 {
    struct {
        u64 X;
        u64 Y;
    };
    struct {
        u64 Min;
        u64 Max;
    };
    u64 V[2];
} u64v2;

// NOTE(acol): 2d intiger vector 2, mostly for rectangles of pixels and whatnot
typedef union i2v2 {
    struct {
        iv2 Min;
        iv2 Max;
    };
    struct {
        iv2 P0;
        iv2 P1;
    };
    iv2 P[2];
    i32 V[4];
} i2v2;

#endif  // BASIC_TYPES_H
