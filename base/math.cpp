// TODO(acol): Remove libc dependency
#include <math.h>

static f32 Sqrt(f32 Val) { return sqrtf(Val); }
static f32 Sin(f32 Val) { return sinf(Val); }
static f32 Cos(f32 Val) { return cosf(Val); }
static f32 Tan(f32 Val) { return tanf(Val); }

static f32 Abs(f32 Value) {
    union {
        f32 F;
        u64 U;
    } Ret;
    Ret.F = Value;
    Ret.U &= 0x7fffffff;
    return Ret.F;
}

static f32 Lerp(f32 Start, f32 End, f32 T) { return (Start + (End - Start) * T); }
static f32 UnLerp(f32 Start, f32 End, f32 X) {
    f32 T = 0.f;
    if (Start != End) {
        T = (X - Start) / (End - Start);
    }
    return (T);
}

static f64 Sqrt(f64 Val) { return sqrt(Val); }
static f64 Sin(f64 Val) { return sin(Val); }
static f64 Cos(f64 Val) { return cos(Val); }
static f64 Tan(f64 Val) { return tan(Val); }

static f64 Abs(f64 Value) {
    union {
        f64 F;
        u64 U;
    } Ret;
    Ret.F = Value;
    Ret.U &= 0x7fffffffffffffffllu;
    return Ret.F;
}

static f64 Lerp(f64 Start, f64 End, f64 T) { return (Start + (End - Start) * T); }

static f64 UnLerp(f64 Start, f64 End, f64 X) {
    f64 T = 0.;
    if (Start != End) {
        T = (X - Start) / (End - Start);
    }
    return (T);
}

static v2 operator+(const v2 A, const v2 B) { return {{A.X + B.X, A.Y + B.Y}}; }
static v3 operator+(const v3 A, const v3 B) { return {{A.X + B.X, A.Y + B.Y, A.Z + B.Z}}; }
static v4 operator+(const v4 A, const v4 B) { return {{A.X + B.X, A.Y + B.Y, A.Z + B.Z, A.W + B.W}}; }
static iv2 operator+(const iv2 A, const iv2 B) { return {{A.X + B.X, A.Y + B.Y}}; }

static v2 operator-(const v2 A, const v2 B) { return {{A.X - B.X, A.Y - B.Y}}; }
static v3 operator-(const v3 A, const v3 B) { return {{A.X - B.X, A.Y - B.Y, A.Z - B.Z}}; }
static v4 operator-(const v4 A, const v4 B) { return {{A.X - B.X, A.Y - B.Y, A.Z - B.Z, A.W - B.W}}; }
static iv2 operator-(const iv2 A, const iv2 B) { return {{A.X - B.X, A.Y - B.Y}}; }

static v2 operator*(const v2 A, const f32 B) { return {{A.X * B, A.Y * B}}; }
static v3 operator*(const v3 A, const f32 B) { return {{A.X * B, A.Y * B, A.Z * B}}; }
static v4 operator*(const v4 A, const f32 B) { return {{A.X * B, A.Y * B, A.Z * B, A.W * B}}; }
static iv2 operator*(const iv2 A, const i32 B) { return {{A.X * B, A.Y * B}}; }

static v2 operator*(const f32 B, const v2 A) { return {{A.X * B, A.Y * B}}; }
static v3 operator*(const f32 B, const v3 A) { return {{A.X * B, A.Y * B, A.Z * B}}; }
static v4 operator*(const f32 B, const v4 A) { return {{A.X * B, A.Y * B, A.Z * B, A.W * B}}; }
static iv2 operator*(const i32 B, const iv2 A) { return {{A.X * B, A.Y * B}}; }

static f32 Dot(const v2 A, const v2 B) { return (A.X * B.X + A.Y * B.Y); }
static f32 Dot(const v3 A, const v3 B) { return (A.X * B.X + A.Y * B.Y + A.Z * B.Z); }
static f32 Dot(const v4 A, const v4 B) { return (A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W); }
static f32 Dot(const iv2 A, const iv2 B) { return (A.X * B.X + A.Y * B.Y); }

static v3 Cross(const v3 A, const v3 B) {
    return {A.Y * B.Z - B.Y * A.Z, B.X * A.Z - A.X * B.Z, A.X * B.Y - B.X * A.Y};
}
