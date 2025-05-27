#ifndef BASE_MATH_H
#define BASE_MATH_H

/*  =====================================================================================

        NOTE(acol): I went fucking bananas with some of these I definitely dont need
        all this shit I need to prune it way the fuck down especially the integer stuff

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

// NOTE(acol): floats
typedef union v2_f32 {
    struct {
        f32 X;
        f32 Y;
    };
    f32 V[2];
} v2_f32;

typedef union v3_f32 {
    struct {
        f32 X;
        f32 Y;
        f32 Z;
    };
    f32 V[3];
} v3_f32;

typedef union v4_f32 {
    struct {
        f32 X;
        f32 Y;
        f32 Z;
        f32 W;
    };
    f32 V[4];
} v4_f32;

typedef union m2_f32 {
    struct {
        v2_f32 Min;
        v2_f32 Max;
    };
    struct {
        v2_f32 P0;
        v2_f32 P1;
    };
    struct {
        v2_f32 R0;
        v2_f32 R1;
    };
    struct {
        f32 A11;
        f32 A12;

        f32 A21;
        f32 A22;
    };
    v2_f32 P[2];
    v2_f32 R[2];
    f32 V[4];
} m2_f32;

typedef union m3_f32 {
    struct {
        v3_f32 P0;
        v3_f32 P1;
        v3_f32 P2;
    };
    struct {
        v3_f32 R0;
        v3_f32 R1;
        v3_f32 R2;
    };
    struct {
        f32 A11;
        f32 A12;
        f32 A13;

        f32 A21;
        f32 A22;
        f32 A23;

        f32 A31;
        f32 A32;
        f32 A33;
    };
    v3_f32 P[3];
    v3_f32 R[3];
    f32 V[9];
} m3_f32;

typedef union m4_f32 {
    struct {
        v4_f32 P0;
        v4_f32 P1;
        v4_f32 P2;
        v4_f32 P3;
    };
    struct {
        v4_f32 R0;
        v4_f32 R1;
        v4_f32 R2;
        v4_f32 R3;
    };
    struct {
        f32 A11;
        f32 A12;
        f32 A13;
        f32 A14;

        f32 A21;
        f32 A22;
        f32 A23;
        f32 A24;

        f32 A31;
        f32 A32;
        f32 A33;
        f32 A34;

        f32 A41;
        f32 A42;
        f32 A43;
        f32 A44;
    };
    v4_f32 P[4];
    v4_f32 R[4];
    f32 V[16];
} m4_f32;

// NOTE(acol): doubles
typedef union v2_f64 {
    struct {
        f64 X;
        f64 Y;
    };
    f64 V[2];
} v2_f64;

typedef union v3_f64 {
    struct {
        f64 X;
        f64 Y;
        f64 Z;
    };
    f64 V[3];
} v3_f64;

typedef union v4_f64 {
    struct {
        f64 X;
        f64 Y;
        f64 Z;
        f64 W;
    };
    f64 V[4];
} v4_f64;

typedef union m2_f64 {
    struct {
        v2_f64 Min;
        v2_f64 Max;
    };
    struct {
        v2_f64 P0;
        v2_f64 P1;
    };
    struct {
        v2_f64 R0;
        v2_f64 R1;
    };
    struct {
        f64 A11;
        f64 A12;

        f64 A21;
        f64 A22;
    };
    v2_f64 P[2];
    v2_f64 R[2];
    f64 V[4];
} m2_f64;

typedef union m3_f64 {
    struct {
        v3_f64 P0;
        v3_f64 P1;
        v3_f64 P2;
    };
    struct {
        v3_f64 R0;
        v3_f64 R1;
        v3_f64 R2;
    };
    struct {
        f64 A11;
        f64 A12;
        f64 A13;

        f64 A21;
        f64 A22;
        f64 A23;

        f64 A31;
        f64 A32;
        f64 A33;
    };
    v3_f64 P[3];
    v3_f64 R[3];
    f64 V[9];
} m3_f64;

typedef union m4_f64 {
    struct {
        v4_f64 P0;
        v4_f64 P1;
        v4_f64 P2;
        v4_f64 P3;
    };
    struct {
        v4_f64 R0;
        v4_f64 R1;
        v4_f64 R2;
        v4_f64 R3;
    };
    struct {
        f64 A11;
        f64 A12;
        f64 A13;
        f64 A14;

        f64 A21;
        f64 A22;
        f64 A23;
        f64 A24;

        f64 A31;
        f64 A32;
        f64 A33;
        f64 A34;

        f64 A41;
        f64 A42;
        f64 A43;
        f64 A44;
    };
    v4_f64 P[4];
    v4_f64 R[4];
    f64 V[16];
} m4_f64;

// NOTE(acol): 16 bit ints
typedef union v2_i16 {
    struct {
        i16 X;
        i16 Y;
    };
    struct {
        i16 Min;
        i16 Max;
    };
    i16 V[2];
} v2_i16;

typedef union v3_i16 {
    struct {
        i16 X;
        i16 Y;
        i16 Z;
    };
    i16 V[3];
} v3_i16;

typedef union v4_i16 {
    struct {
        i16 X;
        i16 Y;
        i16 Z;
        i16 W;
    };
    i16 V[4];
} v4_i16;

typedef union m2_i16 {
    struct {
        v2_i16 Min;
        v2_i16 Max;
    };
    struct {
        v2_i16 P0;
        v2_i16 P1;
    };
    struct {
        v2_i16 R0;
        v2_i16 R1;
    };
    struct {
        i16 A11;
        i16 A12;

        i16 A21;
        i16 A22;
    };
    v2_i16 P[2];
    v2_i16 R[2];
    i16 V[4];
} m2_i16;

typedef union m3_i16 {
    struct {
        v3_i16 P0;
        v3_i16 P1;
        v3_i16 P2;
    };
    struct {
        v3_i16 R0;
        v3_i16 R1;
        v3_i16 R2;
    };
    struct {
        i16 A11;
        i16 A12;
        i16 A13;

        i16 A21;
        i16 A22;
        i16 A23;

        i16 A31;
        i16 A32;
        i16 A33;
    };
    v3_i16 P[3];
    v3_i16 R[3];
    i16 V[9];
} m3_i16;

typedef union m4_i16 {
    struct {
        v4_i16 P0;
        v4_i16 P1;
        v4_i16 P2;
        v4_i16 P3;
    };
    struct {
        v4_i16 R0;
        v4_i16 R1;
        v4_i16 R2;
        v4_i16 R3;
    };
    struct {
        i16 A11;
        i16 A12;
        i16 A13;
        i16 A14;

        i16 A21;
        i16 A22;
        i16 A23;
        i16 A24;

        i16 A31;
        i16 A32;
        i16 A33;
        i16 A34;

        i16 A41;
        i16 A42;
        i16 A43;
        i16 A44;
    };
    v4_i16 P[4];
    v4_i16 R[4];
    i16 V[16];
} m4_i16;

// NOTE(acol): 32 bit ints
typedef union v2_i32 {
    struct {
        i32 X;
        i32 Y;
    };
    struct {
        i32 Min;
        i32 Max;
    };
    i32 V[2];
} v2_i32;

typedef union v3_i32 {
    struct {
        i32 X;
        i32 Y;
        i32 Z;
    };
    i32 V[3];
} v3_i32;

typedef union v4_i32 {
    struct {
        i32 X;
        i32 Y;
        i32 Z;
        i32 W;
    };
    i32 V[4];
} v4_i32;

typedef union m2_i32 {
    struct {
        v2_i32 Min;
        v2_i32 Max;
    };
    struct {
        v2_i32 P0;
        v2_i32 P1;
    };
    struct {
        v2_i32 R0;
        v2_i32 R1;
    };
    struct {
        i32 A11;
        i32 A12;

        i32 A21;
        i32 A22;
    };
    v2_i32 P[2];
    v2_i32 R[2];
    i32 V[4];
} m2_i32;

typedef union m3_i32 {
    struct {
        v3_i32 P0;
        v3_i32 P1;
        v3_i32 P2;
    };
    struct {
        v3_i32 R0;
        v3_i32 R1;
        v3_i32 R2;
    };
    struct {
        i32 A11;
        i32 A12;
        i32 A13;

        i32 A21;
        i32 A22;
        i32 A23;

        i32 A31;
        i32 A32;
        i32 A33;
    };
    v3_i32 P[3];
    v3_i32 R[3];
    i32 V[9];
} m3_i32;

typedef union m4_i32 {
    struct {
        v4_i32 P0;
        v4_i32 P1;
        v4_i32 P2;
        v4_i32 P3;
    };
    struct {
        v4_i32 R0;
        v4_i32 R1;
        v4_i32 R2;
        v4_i32 R3;
    };
    struct {
        i32 A11;
        i32 A12;
        i32 A13;
        i32 A14;

        i32 A21;
        i32 A22;
        i32 A23;
        i32 A24;

        i32 A31;
        i32 A32;
        i32 A33;
        i32 A34;

        i32 A41;
        i32 A42;
        i32 A43;
        i32 A44;
    };
    v4_i32 P[4];
    v4_i32 R[4];
    i32 V[16];
} m4_i32;

// NOTE(acol): 64 bit ints
typedef union v2_i64 {
    struct {
        i64 X;
        i64 Y;
    };
    struct {
        i64 Min;
        i64 Max;
    };
    i64 V[2];
} v2_i64;

typedef union v3_i64 {
    struct {
        i64 X;
        i64 Y;
        i64 Z;
    };
    i64 V[3];
} v3_i64;

typedef union v4_i64 {
    struct {
        i64 X;
        i64 Y;
        i64 Z;
        i64 W;
    };
    i64 V[4];
} v4_i64;

typedef union m2_i64 {
    struct {
        v2_i64 Min;
        v2_i64 Max;
    };
    struct {
        v2_i64 P0;
        v2_i64 P1;
    };
    struct {
        v2_i64 R0;
        v2_i64 R1;
    };
    struct {
        i64 A11;
        i64 A12;

        i64 A21;
        i64 A22;
    };
    v2_i64 P[2];
    v2_i64 R[2];
    i64 V[4];
} m2_i64;

typedef union m3_i64 {
    struct {
        v3_i64 P0;
        v3_i64 P1;
        v3_i64 P2;
    };
    struct {
        v3_i64 R0;
        v3_i64 R1;
        v3_i64 R2;
    };
    struct {
        i64 A11;
        i64 A12;
        i64 A13;

        i64 A21;
        i64 A22;
        i64 A23;

        i64 A31;
        i64 A32;
        i64 A33;
    };
    v3_i64 P[3];
    v3_i64 R[3];
    i64 V[9];
} m3_i64;

typedef union m4_i64 {
    struct {
        v4_i64 P0;
        v4_i64 P1;
        v4_i64 P2;
        v4_i64 P3;
    };
    struct {
        v4_i64 R0;
        v4_i64 R1;
        v4_i64 R2;
        v4_i64 R3;
    };
    struct {
        i64 A11;
        i64 A12;
        i64 A13;
        i64 A14;

        i64 A21;
        i64 A22;
        i64 A23;
        i64 A24;

        i64 A31;
        i64 A32;
        i64 A33;
        i64 A34;

        i64 A41;
        i64 A42;
        i64 A43;
        i64 A44;
    };
    v4_i64 P[4];
    v4_i64 R[4];
    i64 V[16];
} m4_i64;

typedef union v2_u16 {
    struct {
        u16 X;
        u16 Y;
    };
    struct {
        u16 Min;
        u16 Max;
    };
    u16 V[2];
} v2_u16;

typedef union v3_u16 {
    struct {
        u16 X;
        u16 Y;
        u16 Z;
    };
    u16 V[3];
} v3_u16;

typedef union v4_u16 {
    struct {
        u16 X;
        u16 Y;
        u16 Z;
        u16 W;
    };
    u16 V[4];
} v4_u16;

typedef union m2_u16 {
    struct {
        v2_u16 Min;
        v2_u16 Max;
    };
    struct {
        v2_u16 P0;
        v2_u16 P1;
    };
    struct {
        v2_u16 R0;
        v2_u16 R1;
    };
    struct {
        u16 A11;
        u16 A12;

        u16 A21;
        u16 A22;
    };
    v2_u16 P[2];
    v2_u16 R[2];
    u16 V[4];
} m2_u16;

typedef union m3_u16 {
    struct {
        v3_u16 P0;
        v3_u16 P1;
        v3_u16 P2;
    };
    struct {
        v3_u16 R0;
        v3_u16 R1;
        v3_u16 R2;
    };
    struct {
        u16 A11;
        u16 A12;
        u16 A13;

        u16 A21;
        u16 A22;
        u16 A23;

        u16 A31;
        u16 A32;
        u16 A33;
    };
    v3_u16 P[3];
    v3_u16 R[3];
    u16 V[9];
} m3_u16;

typedef union m4_u16 {
    struct {
        v4_u16 P0;
        v4_u16 P1;
        v4_u16 P2;
        v4_u16 P3;
    };
    struct {
        v4_u16 R0;
        v4_u16 R1;
        v4_u16 R2;
        v4_u16 R3;
    };
    struct {
        u16 A11;
        u16 A12;
        u16 A13;
        u16 A14;

        u16 A21;
        u16 A22;
        u16 A23;
        u16 A24;

        u16 A31;
        u16 A32;
        u16 A33;
        u16 A34;

        u16 A41;
        u16 A42;
        u16 A43;
        u16 A44;
    };
    v4_u16 P[4];
    v4_u16 R[4];
    u16 V[16];
} m4_u16;

// NOTE(acol): 32 bit ints
typedef union v2_u32 {
    struct {
        u32 X;
        u32 Y;
    };
    struct {
        u32 Min;
        u32 Max;
    };
    struct {
        u32 Width;
        u32 Height;
    };
    u32 V[2];
} v2_u32;

typedef union v3_u32 {
    struct {
        u32 X;
        u32 Y;
        u32 Z;
    };
    u32 V[3];
} v3_u32;

typedef union v4_u32 {
    struct {
        u32 X;
        u32 Y;
        u32 Z;
        u32 W;
    };
    u32 V[4];
} v4_u32;

typedef union m2_u32 {
    struct {
        v2_u32 Min;
        v2_u32 Max;
    };
    struct {
        v2_u32 P0;
        v2_u32 P1;
    };
    struct {
        v2_u32 R0;
        v2_u32 R1;
    };
    struct {
        u32 A11;
        u32 A12;

        u32 A21;
        u32 A22;
    };
    v2_u32 P[2];
    v2_u32 R[2];
    u32 V[4];
} m2_u32;

typedef union m3_u32 {
    struct {
        v3_u32 P0;
        v3_u32 P1;
        v3_u32 P2;
    };
    struct {
        v3_u32 R0;
        v3_u32 R1;
        v3_u32 R2;
    };
    struct {
        u32 A11;
        u32 A12;
        u32 A13;

        u32 A21;
        u32 A22;
        u32 A23;

        u32 A31;
        u32 A32;
        u32 A33;
    };
    v3_u32 P[3];
    v3_u32 R[3];
    u32 V[9];
} m3_u32;

typedef union m4_u32 {
    struct {
        v4_u32 P0;
        v4_u32 P1;
        v4_u32 P2;
        v4_u32 P3;
    };
    struct {
        v4_u32 R0;
        v4_u32 R1;
        v4_u32 R2;
        v4_u32 R3;
    };
    struct {
        u32 A11;
        u32 A12;
        u32 A13;
        u32 A14;

        u32 A21;
        u32 A22;
        u32 A23;
        u32 A24;

        u32 A31;
        u32 A32;
        u32 A33;
        u32 A34;

        u32 A41;
        u32 A42;
        u32 A43;
        u32 A44;
    };
    v4_u32 P[4];
    v4_u32 R[4];
    u32 V[16];
} m4_u32;

// NOTE(acol): 64 bit ints
typedef union v2_u64 {
    struct {
        u64 X;
        u64 Y;
    };
    struct {
        u64 Min;
        u64 Max;
    };
    u64 V[2];
} v2_u64;

typedef union v3_u64 {
    struct {
        u64 X;
        u64 Y;
        u64 Z;
    };
    u64 V[3];
} v3_u64;

typedef union v4_u64 {
    struct {
        u64 X;
        u64 Y;
        u64 Z;
        u64 W;
    };
    u64 V[4];
} v4_u64;

typedef union m2_u64 {
    struct {
        v2_u64 Min;
        v2_u64 Max;
    };
    struct {
        v2_u64 P0;
        v2_u64 P1;
    };
    struct {
        v2_u64 R0;
        v2_u64 R1;
    };
    struct {
        u64 A11;
        u64 A12;

        u64 A21;
        u64 A22;
    };
    v2_u64 P[2];
    v2_u64 R[2];
    u64 V[4];
} m2_u64;

typedef union m3_u64 {
    struct {
        v3_u64 P0;
        v3_u64 P1;
        v3_u64 P2;
    };
    struct {
        v3_u64 R0;
        v3_u64 R1;
        v3_u64 R2;
    };
    struct {
        u64 A11;
        u64 A12;
        u64 A13;

        u64 A21;
        u64 A22;
        u64 A23;

        u64 A31;
        u64 A32;
        u64 A33;
    };
    v3_u64 P[3];
    v3_u64 R[3];
    u64 V[9];
} m3_u64;

typedef union m4_u64 {
    struct {
        v4_u64 P0;
        v4_u64 P1;
        v4_u64 P2;
        v4_u64 P3;
    };
    struct {
        v4_u64 R0;
        v4_u64 R1;
        v4_u64 R2;
        v4_u64 R3;
    };
    struct {
        u64 A11;
        u64 A12;
        u64 A13;
        u64 A14;

        u64 A21;
        u64 A22;
        u64 A23;
        u64 A24;

        u64 A31;
        u64 A32;
        u64 A33;
        u64 A34;

        u64 A41;
        u64 A42;
        u64 A43;
        u64 A44;
    };
    v4_u64 P[4];
    v4_u64 R[4];
    u64 V[16];
} m4_u64;

#define SqrtF32(Val) sqrtf(Val)
#define SinF32(Val) sinf(Val)
#define CosF32(Val) cosf(Val)
#define TanF32(Val) tanf(Val)

#define SqrtF64(Val) sqrt(Val)
#define SinF64(Val) sin(Val)
#define CosF64(Val) cos(Val)
#define TanF64(Val) tan(Val)

static f32 AbsF32(f32 Value);
static f32 LerpF32(f32 Start, f32 End, f32 T);
static f32 UnLerpF32(f32 Start, f32 End, f32 X);

static f64 AbsF64(f64 Value);
static f64 LerpF64(f64 Start, f64 End, f64 T);
static f64 UnLerpF64(f64 Start, f64 End, f64 X);

static i16 AbsI16(i16 Value);
static i16 LerpI16(i16 Start, i16 End, f32 T);

static i32 AbsI32(i32 Value);
static i32 LerpI32(i32 Start, i32 End, f32 T);

static i64 AbsI64(i64 Value);
static i64 LerpI64(i64 Start, i64 End, f32 T);

static u16 LerpU16(u16 Start, u16 End, f32 T);

static u32 LerpU32(u32 Start, u32 End, f32 T);

static u64 LerpU64(u64 Start, u64 End, f32 T);

// NOTE(acol): vectors
static v2_f32 AddV2F32(v2_f32 A, v2_f32 B);
static v2_f32 SubV2F32(v2_f32 A, v2_f32 B);
static v2_f32 MulV2F32(v2_f32 A, v2_f32 B);
static v2_f32 DivV2F32(v2_f32 A, v2_f32 B);
static v2_f32 ScaleV2F32(v2_f32 V, f32 S);
static f32 DotV2F32(v2_f32 A, v2_f32 B);
static f32 LengthSquaredV2F32(v2_f32 V);
static f32 LengthV2F32(v2_f32 V);
static v2_f32 NormalizeV2F32(v2_f32 V);
static v2_f32 LerpV2F32(v2_f32 Start, v2_f32 End, f32 T);

static v3_f32 AddV3F32(v3_f32 A, v3_f32 B);
static v3_f32 SubV3F32(v3_f32 A, v3_f32 B);
static v3_f32 MulV3F32(v3_f32 A, v3_f32 B);
static v3_f32 DivV3F32(v3_f32 A, v3_f32 B);
static v3_f32 ScaleV3F32(v3_f32 V, f32 S);
static f32 DotV3F32(v3_f32 A, v3_f32 B);
static f32 LengthSquaredV3F32(v3_f32 V);
static f32 LengthV3F32(v3_f32 V);
static v3_f32 NormalizeV3F32(v3_f32 V);
static v3_f32 LerpV3F32(v3_f32 Start, v3_f32 End, f32 T);
static v3_f32 CrossV3F32(v3_f32 A, v3_f32 B);

static v4_f32 AddV4F32(v4_f32 A, v4_f32 B);
static v4_f32 SubV4F32(v4_f32 A, v4_f32 B);
static v4_f32 MulV4F32(v4_f32 A, v4_f32 B);
static v4_f32 DivV4F32(v4_f32 A, v4_f32 B);
static v4_f32 ScaleV4F32(v4_f32 V, f32 S);
static f32 DotV4F32(v4_f32 A, v4_f32 B);
static f32 LengthSquaredV4F32(v4_f32 V);
static f32 LengthV4F32(v4_f32 V);
static v4_f32 NormalizeV4F32(v4_f32 V);
static v4_f32 LerpV4F32(v4_f32 Start, v4_f32 End, f32 T);

static v2_f64 AddV2F64(v2_f64 A, v2_f64 B);
static v2_f64 SubV2F64(v2_f64 A, v2_f64 B);
static v2_f64 MulV2F64(v2_f64 A, v2_f64 B);
static v2_f64 DivV2F64(v2_f64 A, v2_f64 B);
static v2_f64 ScaleV2F64(v2_f64 V, f64 S);
static f64 DotV2F64(v2_f64 A, v2_f64 B);
static f64 LengthSquaredV2F64(v2_f64 V);
static f64 LengthV2F64(v2_f64 V);
static v2_f64 NormalizeV2F64(v2_f64 V);
static v2_f64 LerpV2F64(v2_f64 Start, v2_f64 End, f64 T);

static v3_f64 AddV3F64(v3_f64 A, v3_f64 B);
static v3_f64 SubV3F64(v3_f64 A, v3_f64 B);
static v3_f64 MulV3F64(v3_f64 A, v3_f64 B);
static v3_f64 DivV3F64(v3_f64 A, v3_f64 B);
static v3_f64 ScaleV3F64(v3_f64 V, f64 S);
static f64 DotV3F64(v3_f64 A, v3_f64 B);
static f64 LengthSquaredV3F64(v3_f64 V);
static f64 LengthV3F64(v3_f64 V);
static v3_f64 NormalizeV3F64(v3_f64 V);
static v3_f64 LerpV3F64(v3_f64 Start, v3_f64 End, f64 T);
static v3_f64 CrossV3F64(v3_f64 A, v3_f64 B);

static v4_f64 AddV4F64(v4_f64 A, v4_f64 B);
static v4_f64 SubV4F64(v4_f64 A, v4_f64 B);
static v4_f64 MulV4F64(v4_f64 A, v4_f64 B);
static v4_f64 DivV4F64(v4_f64 A, v4_f64 B);
static v4_f64 ScaleV4F64(v4_f64 V, f64 S);
static f64 DotV4F64(v4_f64 A, v4_f64 B);
static f64 LengthSquaredV4F64(v4_f64 V);
static f64 LengthV4F64(v4_f64 V);
static v4_f64 NormalizeV4F64(v4_f64 V);
static v4_f64 LerpV4F64(v4_f64 Start, v4_f64 End, f64 T);

static v2_i16 AddV2I16(v2_i16 A, v2_i16 B);
static v2_i16 SubV2I16(v2_i16 A, v2_i16 B);
static v2_i16 MulV2I16(v2_i16 A, v2_i16 B);
static v2_i16 DivV2I16(v2_i16 A, v2_i16 B);
static v2_i16 ScaleV2I16(v2_i16 V, i16 S);
static i16 DotV2I16(v2_i16 A, v2_i16 B);
static v2_i16 LerpV2I16(v2_i16 Start, v2_i16 End, f32 T);

static v3_i16 AddV3I16(v3_i16 A, v3_i16 B);
static v3_i16 SubV3I16(v3_i16 A, v3_i16 B);
static v3_i16 MulV3I16(v3_i16 A, v3_i16 B);
static v3_i16 DivV3I16(v3_i16 A, v3_i16 B);
static v3_i16 ScaleV3I16(v3_i16 V, i16 S);
static i16 DotV3I16(v3_i16 A, v3_i16 B);
static v3_i16 LerpV3I16(v3_i16 Start, v3_i16 End, f32 T);
static v3_i16 CrossV3I16(v3_i16 A, v3_i16 B);

static v4_i16 AddV4I16(v4_i16 A, v4_i16 B);
static v4_i16 SubV4I16(v4_i16 A, v4_i16 B);
static v4_i16 MulV4I16(v4_i16 A, v4_i16 B);
static v4_i16 DivV4I16(v4_i16 A, v4_i16 B);
static v4_i16 ScaleV4I16(v4_i16 V, i16 S);
static i16 DotV4I16(v4_i16 A, v4_i16 B);
static v4_i16 LerpV4I16(v4_i16 Start, v4_i16 End, f32 T);

static v2_i32 AddV2I32(v2_i32 A, v2_i32 B);
static v2_i32 SubV2I32(v2_i32 A, v2_i32 B);
static v2_i32 MulV2I32(v2_i32 A, v2_i32 B);
static v2_i32 DivV2I32(v2_i32 A, v2_i32 B);
static v2_i32 ScaleV2I32(v2_i32 V, i32 S);
static i32 DotV2I32(v2_i32 A, v2_i32 B);
static v2_i32 LerpV2I32(v2_i32 Start, v2_i32 End, f32 T);

static v3_i32 AddV3I32(v3_i32 A, v3_i32 B);
static v3_i32 SubV3I32(v3_i32 A, v3_i32 B);
static v3_i32 MulV3I32(v3_i32 A, v3_i32 B);
static v3_i32 DivV3I32(v3_i32 A, v3_i32 B);
static v3_i32 ScaleV3I32(v3_i32 V, i32 S);
static i32 DotV3I32(v3_i32 A, v3_i32 B);
static v3_i32 LerpV3I32(v3_i32 Start, v3_i32 End, f32 T);
static v3_i32 CrossV3I32(v3_i32 A, v3_i32 B);

static v4_i32 AddV4I32(v4_i32 A, v4_i32 B);
static v4_i32 SubV4I32(v4_i32 A, v4_i32 B);
static v4_i32 MulV4I32(v4_i32 A, v4_i32 B);
static v4_i32 DivV4I32(v4_i32 A, v4_i32 B);
static v4_i32 ScaleV4I32(v4_i32 V, i32 S);
static i32 DotV4I32(v4_i32 A, v4_i32 B);
static v4_i32 LerpV4I32(v4_i32 Start, v4_i32 End, f32 T);

static v2_i64 AddV2I64(v2_i64 A, v2_i64 B);
static v2_i64 SubV2I64(v2_i64 A, v2_i64 B);
static v2_i64 MulV2I64(v2_i64 A, v2_i64 B);
static v2_i64 DivV2I64(v2_i64 A, v2_i64 B);
static v2_i64 ScaleV2I64(v2_i64 V, i64 S);
static i64 DotV2I64(v2_i64 A, v2_i64 B);
static v2_i64 LerpV2I64(v2_i64 Start, v2_i64 End, f64 T);

static v3_i64 AddV3I64(v3_i64 A, v3_i64 B);
static v3_i64 SubV3I64(v3_i64 A, v3_i64 B);
static v3_i64 MulV3I64(v3_i64 A, v3_i64 B);
static v3_i64 DivV3I64(v3_i64 A, v3_i64 B);
static v3_i64 ScaleV3I64(v3_i64 V, i64 S);
static i64 DotV3I64(v3_i64 A, v3_i64 B);
static v3_i64 LerpV3I64(v3_i64 Start, v3_i64 End, f64 T);
static v3_i64 CrossV3I64(v3_i64 A, v3_i64 B);

static v4_i64 AddV4I64(v4_i64 A, v4_i64 B);
static v4_i64 SubV4I64(v4_i64 A, v4_i64 B);
static v4_i64 MulV4I64(v4_i64 A, v4_i64 B);
static v4_i64 DivV4I64(v4_i64 A, v4_i64 B);
static v4_i64 ScaleV4I64(v4_i64 V, i64 S);
static i64 DotV4I64(v4_i64 A, v4_i64 B);
static v4_i64 LerpV4I64(v4_i64 Start, v4_i64 End, f64 T);
// -----------------------------------

static v2_u16 AddV2U16(v2_u16 A, v2_u16 B);
static v2_u16 SubV2U16(v2_u16 A, v2_u16 B);
static v2_u16 MulV2U16(v2_u16 A, v2_u16 B);
static v2_u16 DivV2U16(v2_u16 A, v2_u16 B);
static v2_u16 ScaleV2U16(v2_u16 V, u16 S);
static u16 DotV2U16(v2_u16 A, v2_u16 B);
static v2_u16 LerpV2U16(v2_u16 Start, v2_u16 End, f32 T);

static v3_u16 AddV3U16(v3_u16 A, v3_u16 B);
static v3_u16 SubV3U16(v3_u16 A, v3_u16 B);
static v3_u16 MulV3U16(v3_u16 A, v3_u16 B);
static v3_u16 DivV3U16(v3_u16 A, v3_u16 B);
static v3_u16 ScaleV3U16(v3_u16 V, u16 S);
static u16 DotV3U16(v3_u16 A, v3_u16 B);
static v3_u16 LerpV3U16(v3_u16 Start, v3_u16 End, f32 T);
static v3_u16 CrossV3U16(v3_u16 A, v3_u16 B);

static v4_u16 AddV4U16(v4_u16 A, v4_u16 B);
static v4_u16 SubV4U16(v4_u16 A, v4_u16 B);
static v4_u16 MulV4U16(v4_u16 A, v4_u16 B);
static v4_u16 DivV4U16(v4_u16 A, v4_u16 B);
static v4_u16 ScaleV4U16(v4_u16 V, u16 S);
static u16 DotV4U16(v4_u16 A, v4_u16 B);
static v4_u16 LerpV4U16(v4_u16 Start, v4_u16 End, f32 T);

static v2_u32 AddV2U32(v2_u32 A, v2_u32 B);
static v2_u32 SubV2U32(v2_u32 A, v2_u32 B);
static v2_u32 MulV2U32(v2_u32 A, v2_u32 B);
static v2_u32 DivV2U32(v2_u32 A, v2_u32 B);
static v2_u32 ScaleV2U32(v2_u32 V, u32 S);
static u32 DotV2U32(v2_u32 A, v2_u32 B);
static v2_u32 LerpV2U32(v2_u32 Start, v2_u32 End, f32 T);

static v3_u32 AddV3U32(v3_u32 A, v3_u32 B);
static v3_u32 SubV3U32(v3_u32 A, v3_u32 B);
static v3_u32 MulV3U32(v3_u32 A, v3_u32 B);
static v3_u32 DivV3U32(v3_u32 A, v3_u32 B);
static v3_u32 ScaleV3U32(v3_u32 V, u32 S);
static u32 DotV3U32(v3_u32 A, v3_u32 B);
static v3_u32 LerpV3U32(v3_u32 Start, v3_u32 End, f32 T);
static v3_u32 CrossV3U32(v3_u32 A, v3_u32 B);

static v4_u32 AddV4U32(v4_u32 A, v4_u32 B);
static v4_u32 SubV4U32(v4_u32 A, v4_u32 B);
static v4_u32 MulV4U32(v4_u32 A, v4_u32 B);
static v4_u32 DivV4U32(v4_u32 A, v4_u32 B);
static v4_u32 ScaleV4U32(v4_u32 V, u32 S);
static u32 DotV4U32(v4_u32 A, v4_u32 B);
static v4_u32 LerpV4U32(v4_u32 Start, v4_u32 End, f32 T);

static v2_u64 AddV2U64(v2_u64 A, v2_u64 B);
static v2_u64 SubV2U64(v2_u64 A, v2_u64 B);
static v2_u64 MulV2U64(v2_u64 A, v2_u64 B);
static v2_u64 DivV2U64(v2_u64 A, v2_u64 B);
static v2_u64 ScaleV2U64(v2_u64 V, u64 S);
static u64 DotV2U64(v2_u64 A, v2_u64 B);
static v2_u64 LerpV2U64(v2_u64 Start, v2_u64 End, f64 T);

static v3_u64 AddV3U64(v3_u64 A, v3_u64 B);
static v3_u64 SubV3U64(v3_u64 A, v3_u64 B);
static v3_u64 MulV3U64(v3_u64 A, v3_u64 B);
static v3_u64 DivV3U64(v3_u64 A, v3_u64 B);
static v3_u64 ScaleV3U64(v3_u64 V, u64 S);
static u64 DotV3U64(v3_u64 A, v3_u64 B);
static v3_u64 LerpV3U64(v3_u64 Start, v3_u64 End, f64 T);
static v3_u64 CrossV3U64(v3_u64 A, v3_u64 B);

static v4_u64 AddV4U64(v4_u64 A, v4_u64 B);
static v4_u64 SubV4U64(v4_u64 A, v4_u64 B);
static v4_u64 MulV4U64(v4_u64 A, v4_u64 B);
static v4_u64 DivV4U64(v4_u64 A, v4_u64 B);
static v4_u64 ScaleV4U64(v4_u64 V, u64 S);
static u64 DotV4U64(v4_u64 A, v4_u64 B);
static v4_u64 LerpV4U64(v4_u64 Start, v4_u64 End, f64 T);

// TODO(acol): matrices arent done will add as I find things midding

static m2_f32 MulM2F32(m2_f32 A, m2_f32 B);
static m3_f32 MulM3F32(m3_f32 A, m3_f32 B);
static m4_f32 MulM4F32(m4_f32 A, m4_f32 B);

static m2_f64 MulM2F64(m2_f64 A, m2_f64 B);
static m3_f64 MulM3F64(m3_f64 A, m3_f64 B);
static m4_f64 MulM4F64(m4_f64 A, m4_f64 B);

#endif  // BASE_MATH_H
