#ifndef MATH_H
#define MATH_H

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
static const f64 EF64 = (f64)2.71828182846;

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

typedef union m2 {
    struct {
        v2 Min;
        v2 Max;
    };
    struct {
        v2 P0;
        v2 P1;
    };
    struct {
        v2 R0;
        v2 R1;
    };
    struct {
        f32 X0;
        f32 Y0;

        f32 X1;
        f32 Y1;
    };
    v2 P[2];
    v2 R[2];
    f32 V[4];
} m2;

typedef union m3 {
    struct {
        v3 P0;
        v3 P1;
        v3 P2;
    };
    struct {
        v3 R0;
        v3 R1;
        v3 R2;
    };
    struct {
        f32 X0;
        f32 Y0;
        f32 Z0;

        f32 X1;
        f32 Y1;
        f32 Z1;

        f32 X2;
        f32 Y2;
        f32 Z2;
    };
    v3 P[3];
    v3 R[3];
    f32 V[9];
} m3;

typedef union m4 {
    struct {
        v4 P0;
        v4 P1;
        v4 P2;
        v4 P3;
    };
    struct {
        v4 R0;
        v4 R1;
        v4 R2;
        v4 R3;
    };
    struct {
        f32 X0;
        f32 Y0;
        f32 Z0;
        f32 W0;

        f32 X1;
        f32 Y1;
        f32 Z1;
        f32 W1;

        f32 X2;
        f32 Y2;
        f32 Z2;
        f32 W2;

        f32 X3;
        f32 Y3;
        f32 Z3;
        f32 W3;
    };
    v4 P[4];
    v4 R[4];
    f32 V[16];
} m4;

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

// NOTE(acol): intiger matrix 2x2, mostly for rectangles of pixels and whatnot
typedef union im2 {
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
} im2;

static f32 Sqrt(f32 Val);
static f32 Sin(f32 Val);
static f32 Cos(f32 Val);
static f32 Tan(f32 Val);
static f32 Abs(f32 Value);
static f32 Lerp(f32 Start, f32 End, f32 T);
static f32 UnLerp(f32 Start, f32 End, f32 X);
static f64 Sqrt(f64 Val);
static f64 Sin(f64 Val);
static f64 Cos(f64 Val);
static f64 Tan(f64 Val);
static f64 Abs(f64 Value);
static f64 Lerp(f64 Start, f64 End, f64 T);
static f64 UnLerp(f64 Start, f64 End, f64 X);

static v2 operator+(const v2 A, const v2 B);
static v3 operator+(const v3 A, const v3 B);
static v4 operator+(const v4 A, const v4 B);
static iv2 operator+(const iv2 A, const iv2 B);

static v2 operator-(const v2 A, const v2 B);
static v3 operator-(const v3 A, const v3 B);
static v4 operator-(const v4 A, const v4 B);
static iv2 operator-(const iv2 A, const iv2 B);

static v2 operator*(const v2 A, const f32 B);
static v3 operator*(const v3 A, const f32 B);
static v4 operator*(const v4 A, const f32 B);
static iv2 operator*(const iv2 A, const i32 B);

static v2 operator*(const f32 B, const v2 A);
static v3 operator*(const f32 B, const v3 A);
static v4 operator*(const f32 B, const v4 A);
static iv2 operator*(const i32 B, const iv2 A);

#endif  // MATH_H
