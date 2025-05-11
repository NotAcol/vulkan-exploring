#include <math.h>

static f32 AbsF32(f32 Value) {
    union {
        f32 F;
        u32 U;
    } Ret;
    Ret.F = Value;
    Ret.U &= BITMASK31;
    return Ret.F;
}
static f32 LerpF32(f32 Start, f32 End, f32 T) { return (Start + (End - Start) * Clamp(0.f, T, 1.f)); }
static f32 UnLerpF32(f32 Start, f32 End, f32 X) {
    f32 T = 0.f;
    if (Start != End) {
        T = (X - Start) / (End - Start);
    }
    return (T);
}

static f64 AbsF64(f64 Value) {
    union {
        f64 F;
        u64 U;
    } Ret;
    Ret.F = Value;
    Ret.U &= BITMASK63;
    return Ret.F;
}
static f64 LerpF64(f64 Start, f64 End, f64 T) { return (Start + (End - Start) * Clamp(0.0, T, 1.0)); }
static f64 UnLerpF64(f64 Start, f64 End, f64 X) {
    f64 T = 0.f;
    if (Start != End) {
        T = (X - Start) / (End - Start);
    }
    return (T);
}

static i16 AbsI16(i16 Value) { return (Value & BITMASK15); }
static i16 LerpI16(i16 Start, i16 End, f32 T) { return (Start + (End - Start) * Clamp(0.f, T, 1.f)); }

static i32 AbsI32(i32 Value) { return (Value & BITMASK31); }
static i32 LerpI32(i32 Start, i32 End, f32 T) { return (Start + (End - Start) * Clamp(0.f, T, 1.f)); }

static i64 AbsI64(i64 Value) { return (Value & BITMASK15); }
static i64 LerpI64(i64 Start, i64 End, f32 T) { return (Start + (End - Start) * Clamp(0.f, T, 1.f)); }

/*  =====================================================================================

                        NOTE(acol): vector stuff

    ===================================================================================== */

static v2_f32 AddV2F32(v2_f32 A, v2_f32 B) { return ((v2_f32){A.X + B.X, A.Y + B.Y}); }
static v2_f32 SubV2F32(v2_f32 A, v2_f32 B) { return ((v2_f32){A.X - B.X, A.Y - B.Y}); }
static v2_f32 MulV2F32(v2_f32 A, v2_f32 B) { return ((v2_f32){A.X * B.X, A.Y * B.Y}); }
static v2_f32 DivV2F32(v2_f32 A, v2_f32 B) { return ((v2_f32){A.X / B.X, A.Y / B.Y}); }
static v2_f32 ScaleV2F32(v2_f32 V, f32 S) { return ((v2_f32){V.X * S, V.Y * S}); }
static f32 DotV2F32(v2_f32 A, v2_f32 B) {
    f32 Ret = A.X * B.X + A.Y * B.Y;
    return Ret;
}
static f32 LengthSquaredV2F32(v2_f32 V) {
    f32 Ret = V.X * V.X + V.Y * V.Y;
    return Ret;
}
static f32 LengthV2F32(v2_f32 V) {
    f32 Ret = SqrtF32(V.X * V.X + V.Y * V.Y);
    return Ret;
}
static v2_f32 NormalizeV2F32(v2_f32 V) {
    v2_f32 Ret = ScaleV2F32(V, 1.f / LengthV2F32(V));
    return Ret;
}
static v2_f32 LerpV2F32(v2_f32 Start, v2_f32 End, f32 T) {
    return ((v2_f32){LerpF32(Start.X, End.X, T), LerpF32(Start.Y, End.Y, T)});
}

static v3_f32 AddV3F32(v3_f32 A, v3_f32 B) { return ((v3_f32){A.X + B.X, A.Y + B.Y, A.Z + B.Z}); }
static v3_f32 SubV3F32(v3_f32 A, v3_f32 B) { return ((v3_f32){A.X - B.X, A.Y - B.Y, A.Z - B.Z}); }
static v3_f32 MulV3F32(v3_f32 A, v3_f32 B) { return ((v3_f32){A.X * B.X, A.Y * B.Y, A.Z * B.Z}); }
static v3_f32 DivV3F32(v3_f32 A, v3_f32 B) { return ((v3_f32){A.X / B.X, A.Y / B.Y, A.Z / B.Z}); }
static v3_f32 ScaleV3F32(v3_f32 V, f32 S) { return ((v3_f32){V.X * S, V.Y * S, V.Z * S}); }
static f32 DotV3F32(v3_f32 A, v3_f32 B) {
    f32 Ret = A.X * B.X + A.Y * B.Y + A.Z * B.Z;
    return Ret;
}
static f32 LengthSquaredV3F32(v3_f32 V) {
    f32 Ret = V.X * V.X + V.Y * V.Y + V.Z * V.Z;
    return Ret;
}
static f32 LengthV3F32(v3_f32 V) {
    f32 Ret = SqrtF32(V.X * V.X + V.Y * V.Y + V.Z * V.Z);
    return Ret;
}
static v3_f32 NormalizeV3F32(v3_f32 V) {
    v3_f32 Ret = ScaleV3F32(V, 1.f / LengthV3F32(V));
    return Ret;
}
static v3_f32 LerpV3F32(v3_f32 Start, v3_f32 End, f32 T) {
    return ((v3_f32){LerpF32(Start.X, End.X, T), LerpF32(Start.Y, End.Y, T), LerpF32(Start.Z, End.Z, T)});
}
static v3_f32 CrossV3F32(v3_f32 A, v3_f32 B) {
    v3_f32 Ret = (v3_f32){A.Y * B.Z - A.Z * B.Y, A.Z * B.X - A.X * B.Z, A.X * B.Y - A.Y * B.X};
    return Ret;
}

static v4_f32 AddV4F32(v4_f32 A, v4_f32 B) { return ((v4_f32){A.X + B.X, A.Y + B.Y, A.Z + B.Z, A.W + B.W}); }
static v4_f32 SubV4F32(v4_f32 A, v4_f32 B) { return ((v4_f32){A.X - B.X, A.Y - B.Y, A.Z - B.Z, A.W - B.W}); }
static v4_f32 MulV4F32(v4_f32 A, v4_f32 B) { return ((v4_f32){A.X * B.X, A.Y * B.Y, A.Z * B.Z, A.W * B.W}); }
static v4_f32 DivV4F32(v4_f32 A, v4_f32 B) { return ((v4_f32){A.X / B.X, A.Y / B.Y, A.Z / B.Z, A.W / B.W}); }
static v4_f32 ScaleV4F32(v4_f32 V, f32 S) { return ((v4_f32){V.X * S, V.Y * S, V.Z * S, V.W * S}); }
static f32 DotV4F32(v4_f32 A, v4_f32 B) {
    f32 Ret = A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W;
    return Ret;
}
static f32 LengthSquaredV4F32(v4_f32 V) {
    f32 Ret = V.X * V.X + V.Y * V.Y + V.Z * V.Z + V.W * V.W;
    return Ret;
}
static f32 LengthV4F32(v4_f32 V) {
    f32 Ret = SqrtF32(V.X * V.X + V.Y * V.Y + V.Z * V.Z + V.W * V.W);
    return Ret;
}
static v4_f32 NormalizeV4F32(v4_f32 V) {
    v4_f32 Ret = ScaleV4F32(V, 1.f / LengthV4F32(V));
    return Ret;
}
static v4_f32 LerpV4F32(v4_f32 Start, v4_f32 End, f32 T) {
    return ((v4_f32){LerpF32(Start.X, End.X, T), LerpF32(Start.Y, End.Y, T), LerpF32(Start.Z, End.Z, T),
                     LerpF32(Start.W, End.W, T)});
}

static v2_f64 AddV2F64(v2_f64 A, v2_f64 B) { return ((v2_f64){A.X + B.X, A.Y + B.Y}); }
static v2_f64 SubV2F64(v2_f64 A, v2_f64 B) { return ((v2_f64){A.X - B.X, A.Y - B.Y}); }
static v2_f64 MulV2F64(v2_f64 A, v2_f64 B) { return ((v2_f64){A.X * B.X, A.Y * B.Y}); }
static v2_f64 DivV2F64(v2_f64 A, v2_f64 B) { return ((v2_f64){A.X / B.X, A.Y / B.Y}); }
static v2_f64 ScaleV2F64(v2_f64 V, f64 S) { return ((v2_f64){V.X * S, V.Y * S}); }
static f64 DotV2F64(v2_f64 A, v2_f64 B) {
    f64 Ret = A.X * B.X + A.Y * B.Y;
    return Ret;
}
static f64 LengthSquaredV2F64(v2_f64 V) {
    f64 Ret = V.X * V.X + V.Y * V.Y;
    return Ret;
}
static f64 LengthV2F64(v2_f64 V) {
    f64 Ret = SqrtF64(V.X * V.X + V.Y * V.Y);
    return Ret;
}
static v2_f64 NormalizeV2F64(v2_f64 V) {
    v2_f64 Ret = ScaleV2F64(V, 1.0 / LengthV2F64(V));
    return Ret;
}
static v2_f64 LerpV2F64(v2_f64 Start, v2_f64 End, f64 T) {
    return ((v2_f64){LerpF64(Start.X, End.X, T), LerpF64(Start.Y, End.Y, T)});
}

static v3_f64 AddV3F64(v3_f64 A, v3_f64 B) { return ((v3_f64){A.X + B.X, A.Y + B.Y, A.Z + B.Z}); }
static v3_f64 SubV3F64(v3_f64 A, v3_f64 B) { return ((v3_f64){A.X - B.X, A.Y - B.Y, A.Z - B.Z}); }
static v3_f64 MulV3F64(v3_f64 A, v3_f64 B) { return ((v3_f64){A.X * B.X, A.Y * B.Y, A.Z * B.Z}); }
static v3_f64 DivV3F64(v3_f64 A, v3_f64 B) { return ((v3_f64){A.X / B.X, A.Y / B.Y, A.Z / B.Z}); }
static v3_f64 ScaleV3F64(v3_f64 V, f64 S) { return ((v3_f64){V.X * S, V.Y * S, V.Z * S}); }
static f64 DotV3F64(v3_f64 A, v3_f64 B) {
    f64 Ret = A.X * B.X + A.Y * B.Y + A.Z * B.Z;
    return Ret;
}
static f64 LengthSquaredV3F64(v3_f64 V) {
    f64 Ret = V.X * V.X + V.Y * V.Y + V.Z * V.Z;
    return Ret;
}
static f64 LengthV3F64(v3_f64 V) {
    f64 Ret = SqrtF64(V.X * V.X + V.Y * V.Y + V.Z * V.Z);
    return Ret;
}
static v3_f64 NormalizeV3F64(v3_f64 V) {
    v3_f64 Ret = ScaleV3F64(V, 1.0 / LengthV3F64(V));
    return Ret;
}
static v3_f64 LerpV3F64(v3_f64 Start, v3_f64 End, f64 T) {
    return ((v3_f64){LerpF64(Start.X, End.X, T), LerpF64(Start.Y, End.Y, T), LerpF64(Start.Z, End.Z, T)});
}
static v3_f64 CrossV3F64(v3_f64 A, v3_f64 B) {
    v3_f64 Ret = (v3_f64){A.Y * B.Z - A.Z * B.Y, A.Z * B.X - A.X * B.Z, A.X * B.Y - A.Y * B.X};
    return Ret;
}

static v4_f64 AddV4F64(v4_f64 A, v4_f64 B) { return ((v4_f64){A.X + B.X, A.Y + B.Y, A.Z + B.Z, A.W + B.W}); }
static v4_f64 SubV4F64(v4_f64 A, v4_f64 B) { return ((v4_f64){A.X - B.X, A.Y - B.Y, A.Z - B.Z, A.W - B.W}); }
static v4_f64 MulV4F64(v4_f64 A, v4_f64 B) { return ((v4_f64){A.X * B.X, A.Y * B.Y, A.Z * B.Z, A.W * B.W}); }
static v4_f64 DivV4F64(v4_f64 A, v4_f64 B) { return ((v4_f64){A.X / B.X, A.Y / B.Y, A.Z / B.Z, A.W / B.W}); }
static v4_f64 ScaleV4F64(v4_f64 V, f64 S) { return ((v4_f64){V.X * S, V.Y * S, V.Z * S, V.W * S}); }
static f64 DotV4F64(v4_f64 A, v4_f64 B) {
    f64 Ret = A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W;
    return Ret;
}
static f64 LengthSquaredV4F64(v4_f64 V) {
    f64 Ret = V.X * V.X + V.Y * V.Y + V.Z * V.Z + V.W * V.W;
    return Ret;
}
static f64 LengthV4F64(v4_f64 V) {
    f64 Ret = SqrtF64(V.X * V.X + V.Y * V.Y + V.Z * V.Z + V.W * V.W);
    return Ret;
}
static v4_f64 NormalizeV4F64(v4_f64 V) {
    v4_f64 Ret = ScaleV4F64(V, 1.0 / LengthV4F64(V));
    return Ret;
}
static v4_f64 LerpV4F64(v4_f64 Start, v4_f64 End, f64 T) {
    return ((v4_f64){LerpF64(Start.X, End.X, T), LerpF64(Start.Y, End.Y, T), LerpF64(Start.Z, End.Z, T),
                     LerpF64(Start.W, End.W, T)});
}
static v2_i16 AddV2I16(v2_i16 A, v2_i16 B) { return ((v2_i16){A.X + B.X, A.Y + B.Y}); }
static v2_i16 SubV2I16(v2_i16 A, v2_i16 B) { return ((v2_i16){A.X - B.X, A.Y - B.Y}); }
static v2_i16 MulV2I16(v2_i16 A, v2_i16 B) { return ((v2_i16){A.X * B.X, A.Y * B.Y}); }
static v2_i16 DivV2I16(v2_i16 A, v2_i16 B) { return ((v2_i16){A.X / B.X, A.Y / B.Y}); }
static v2_i16 ScaleV2I16(v2_i16 V, i16 S) { return ((v2_i16){V.X * S, V.Y * S}); }
static i16 DotV2I16(v2_i16 A, v2_i16 B) {
    i16 Ret = A.X * B.X + A.Y * B.Y;
    return Ret;
}
static v2_i16 LerpV2I16(v2_i16 Start, v2_i16 End, f32 T) {
    return ((v2_i16){LerpI16(Start.X, End.X, T), LerpI16(Start.Y, End.Y, T)});
}

static v3_i16 AddV3I16(v3_i16 A, v3_i16 B) { return ((v3_i16){A.X + B.X, A.Y + B.Y, A.Z + B.Z}); }
static v3_i16 SubV3I16(v3_i16 A, v3_i16 B) { return ((v3_i16){A.X - B.X, A.Y - B.Y, A.Z - B.Z}); }
static v3_i16 MulV3I16(v3_i16 A, v3_i16 B) { return ((v3_i16){A.X * B.X, A.Y * B.Y, A.Z * B.Z}); }
static v3_i16 DivV3I16(v3_i16 A, v3_i16 B) { return ((v3_i16){A.X / B.X, A.Y / B.Y, A.Z / B.Z}); }
static v3_i16 ScaleV3I16(v3_i16 V, i16 S) { return ((v3_i16){V.X * S, V.Y * S, V.Z * S}); }
static i16 DotV3I16(v3_i16 A, v3_i16 B) {
    i16 Ret = A.X * B.X + A.Y * B.Y + A.Z * B.Z;
    return Ret;
}
static v3_i16 LerpV3I16(v3_i16 Start, v3_i16 End, f32 T) {
    return ((v3_i16){LerpI16(Start.X, End.X, T), LerpI16(Start.Y, End.Y, T), LerpI16(Start.Z, End.Z, T)});
}
static v3_i16 CrossV3I16(v3_i16 A, v3_i16 B) {
    v3_i16 Ret = (v3_i16){A.Y * B.Z - A.Z * B.Y, A.Z * B.X - A.X * B.Z, A.X * B.Y - A.Y * B.X};
    return Ret;
}

static v4_i16 AddV4I16(v4_i16 A, v4_i16 B) { return ((v4_i16){A.X + B.X, A.Y + B.Y, A.Z + B.Z, A.W + B.W}); }
static v4_i16 SubV4I16(v4_i16 A, v4_i16 B) { return ((v4_i16){A.X - B.X, A.Y - B.Y, A.Z - B.Z, A.W - B.W}); }
static v4_i16 MulV4I16(v4_i16 A, v4_i16 B) { return ((v4_i16){A.X * B.X, A.Y * B.Y, A.Z * B.Z, A.W * B.W}); }
static v4_i16 DivV4I16(v4_i16 A, v4_i16 B) { return ((v4_i16){A.X / B.X, A.Y / B.Y, A.Z / B.Z, A.W / B.W}); }
static v4_i16 ScaleV4I16(v4_i16 V, i16 S) { return ((v4_i16){V.X * S, V.Y * S, V.Z * S, V.W * S}); }
static i16 DotV4I16(v4_i16 A, v4_i16 B) {
    i16 Ret = A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W;
    return Ret;
}
static v4_i16 LerpV4I16(v4_i16 Start, v4_i16 End, f32 T) {
    return ((v4_i16){LerpI16(Start.X, End.X, T), LerpI16(Start.Y, End.Y, T), LerpI16(Start.Z, End.Z, T),
                     LerpI16(Start.W, End.W, T)});
}

static v2_i32 AddV2I32(v2_i32 A, v2_i32 B) { return ((v2_i32){A.X + B.X, A.Y + B.Y}); }
static v2_i32 SubV2I32(v2_i32 A, v2_i32 B) { return ((v2_i32){A.X - B.X, A.Y - B.Y}); }
static v2_i32 MulV2I32(v2_i32 A, v2_i32 B) { return ((v2_i32){A.X * B.X, A.Y * B.Y}); }
static v2_i32 DivV2I32(v2_i32 A, v2_i32 B) { return ((v2_i32){A.X / B.X, A.Y / B.Y}); }
static v2_i32 ScaleV2I32(v2_i32 V, i32 S) { return ((v2_i32){V.X * S, V.Y * S}); }
static i32 DotV2I32(v2_i32 A, v2_i32 B) {
    i32 Ret = A.X * B.X + A.Y * B.Y;
    return Ret;
}
static v2_i32 LerpV2I32(v2_i32 Start, v2_i32 End, f32 T) {
    return ((v2_i32){LerpI32(Start.X, End.X, T), LerpI32(Start.Y, End.Y, T)});
}

static v3_i32 AddV3I32(v3_i32 A, v3_i32 B) { return ((v3_i32){A.X + B.X, A.Y + B.Y, A.Z + B.Z}); }
static v3_i32 SubV3I32(v3_i32 A, v3_i32 B) { return ((v3_i32){A.X - B.X, A.Y - B.Y, A.Z - B.Z}); }
static v3_i32 MulV3I32(v3_i32 A, v3_i32 B) { return ((v3_i32){A.X * B.X, A.Y * B.Y, A.Z * B.Z}); }
static v3_i32 DivV3I32(v3_i32 A, v3_i32 B) { return ((v3_i32){A.X / B.X, A.Y / B.Y, A.Z / B.Z}); }
static v3_i32 ScaleV3I32(v3_i32 V, i32 S) { return ((v3_i32){V.X * S, V.Y * S, V.Z * S}); }
static i32 DotV3I32(v3_i32 A, v3_i32 B) {
    i32 Ret = A.X * B.X + A.Y * B.Y + A.Z * B.Z;
    return Ret;
}
static v3_i32 LerpV3I32(v3_i32 Start, v3_i32 End, f32 T) {
    return ((v3_i32){LerpI32(Start.X, End.X, T), LerpI32(Start.Y, End.Y, T), LerpI32(Start.Z, End.Z, T)});
}
static v3_i32 CrossV3I32(v3_i32 A, v3_i32 B) {
    v3_i32 Ret = (v3_i32){A.Y * B.Z - A.Z * B.Y, A.Z * B.X - A.X * B.Z, A.X * B.Y - A.Y * B.X};
    return Ret;
}

static v4_i32 AddV4I32(v4_i32 A, v4_i32 B) { return ((v4_i32){A.X + B.X, A.Y + B.Y, A.Z + B.Z, A.W + B.W}); }
static v4_i32 SubV4I32(v4_i32 A, v4_i32 B) { return ((v4_i32){A.X - B.X, A.Y - B.Y, A.Z - B.Z, A.W - B.W}); }
static v4_i32 MulV4I32(v4_i32 A, v4_i32 B) { return ((v4_i32){A.X * B.X, A.Y * B.Y, A.Z * B.Z, A.W * B.W}); }
static v4_i32 DivV4I32(v4_i32 A, v4_i32 B) { return ((v4_i32){A.X / B.X, A.Y / B.Y, A.Z / B.Z, A.W / B.W}); }
static v4_i32 ScaleV4I32(v4_i32 V, i32 S) { return ((v4_i32){V.X * S, V.Y * S, V.Z * S, V.W * S}); }
static i32 DotV4I32(v4_i32 A, v4_i32 B) {
    i32 Ret = A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W;
    return Ret;
}
static v4_i32 LerpV4I32(v4_i32 Start, v4_i32 End, f32 T) {
    return ((v4_i32){LerpI32(Start.X, End.X, T), LerpI32(Start.Y, End.Y, T), LerpI32(Start.Z, End.Z, T),
                     LerpI32(Start.W, End.W, T)});
}

static v2_i64 AddV2I64(v2_i64 A, v2_i64 B) { return ((v2_i64){A.X + B.X, A.Y + B.Y}); }
static v2_i64 SubV2I64(v2_i64 A, v2_i64 B) { return ((v2_i64){A.X - B.X, A.Y - B.Y}); }
static v2_i64 MulV2I64(v2_i64 A, v2_i64 B) { return ((v2_i64){A.X * B.X, A.Y * B.Y}); }
static v2_i64 DivV2I64(v2_i64 A, v2_i64 B) { return ((v2_i64){A.X / B.X, A.Y / B.Y}); }
static v2_i64 ScaleV2I64(v2_i64 V, i64 S) { return ((v2_i64){V.X * S, V.Y * S}); }
static i64 DotV2I64(v2_i64 A, v2_i64 B) {
    i64 Ret = A.X * B.X + A.Y * B.Y;
    return Ret;
}
static v2_i64 LerpV2I64(v2_i64 Start, v2_i64 End, f64 T) {
    return ((v2_i64){LerpI64(Start.X, End.X, T), LerpI64(Start.Y, End.Y, T)});
}

static v3_i64 AddV3I64(v3_i64 A, v3_i64 B) { return ((v3_i64){A.X + B.X, A.Y + B.Y, A.Z + B.Z}); }
static v3_i64 SubV3I64(v3_i64 A, v3_i64 B) { return ((v3_i64){A.X - B.X, A.Y - B.Y, A.Z - B.Z}); }
static v3_i64 MulV3I64(v3_i64 A, v3_i64 B) { return ((v3_i64){A.X * B.X, A.Y * B.Y, A.Z * B.Z}); }
static v3_i64 DivV3I64(v3_i64 A, v3_i64 B) { return ((v3_i64){A.X / B.X, A.Y / B.Y, A.Z / B.Z}); }
static v3_i64 ScaleV3I64(v3_i64 V, i64 S) { return ((v3_i64){V.X * S, V.Y * S, V.Z * S}); }
static i64 DotV3I64(v3_i64 A, v3_i64 B) {
    i64 Ret = A.X * B.X + A.Y * B.Y + A.Z * B.Z;
    return Ret;
}
static v3_i64 LerpV3I64(v3_i64 Start, v3_i64 End, f64 T) {
    return ((v3_i64){LerpI64(Start.X, End.X, T), LerpI64(Start.Y, End.Y, T), LerpI64(Start.Z, End.Z, T)});
}
static v3_i64 CrossV3I64(v3_i64 A, v3_i64 B) {
    v3_i64 Ret = (v3_i64){A.Y * B.Z - A.Z * B.Y, A.Z * B.X - A.X * B.Z, A.X * B.Y - A.Y * B.X};
    return Ret;
}

static v4_i64 AddV4I64(v4_i64 A, v4_i64 B) { return ((v4_i64){A.X + B.X, A.Y + B.Y, A.Z + B.Z, A.W + B.W}); }
static v4_i64 SubV4I64(v4_i64 A, v4_i64 B) { return ((v4_i64){A.X - B.X, A.Y - B.Y, A.Z - B.Z, A.W - B.W}); }
static v4_i64 MulV4I64(v4_i64 A, v4_i64 B) { return ((v4_i64){A.X * B.X, A.Y * B.Y, A.Z * B.Z, A.W * B.W}); }
static v4_i64 DivV4I64(v4_i64 A, v4_i64 B) { return ((v4_i64){A.X / B.X, A.Y / B.Y, A.Z / B.Z, A.W / B.W}); }
static v4_i64 ScaleV4I64(v4_i64 V, i64 S) { return ((v4_i64){V.X * S, V.Y * S, V.Z * S, V.W * S}); }
static i64 DotV4I64(v4_i64 A, v4_i64 B) {
    i64 Ret = A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W;
    return Ret;
}
static v4_i64 LerpV4I64(v4_i64 Start, v4_i64 End, f64 T) {
    return ((v4_i64){LerpI64(Start.X, End.X, T), LerpI64(Start.Y, End.Y, T), LerpI64(Start.Z, End.Z, T),
                     LerpI64(Start.W, End.W, T)});
}

static u16 LerpU16(u16 Start, u16 End, f32 T) { return (Start + (End - Start) * Clamp(0.f, T, 1.f)); }

static u32 LerpU32(u32 Start, u32 End, f32 T) { return (Start + (End - Start) * Clamp(0.f, T, 1.f)); }

static u64 LerpU64(u64 Start, u64 End, f32 T) { return (Start + (End - Start) * Clamp(0.f, T, 1.f)); }

static v2_u16 AddV2U16(v2_u16 A, v2_u16 B) { return ((v2_u16){A.X + B.X, A.Y + B.Y}); }
static v2_u16 SubV2U16(v2_u16 A, v2_u16 B) { return ((v2_u16){A.X - B.X, A.Y - B.Y}); }
static v2_u16 MulV2U16(v2_u16 A, v2_u16 B) { return ((v2_u16){A.X * B.X, A.Y * B.Y}); }
static v2_u16 DivV2U16(v2_u16 A, v2_u16 B) { return ((v2_u16){A.X / B.X, A.Y / B.Y}); }
static v2_u16 ScaleV2U16(v2_u16 V, u16 S) { return ((v2_u16){V.X * S, V.Y * S}); }
static u16 DotV2U16(v2_u16 A, v2_u16 B) {
    u16 Ret = A.X * B.X + A.Y * B.Y;
    return Ret;
}
static v2_u16 LerpV2U16(v2_u16 Start, v2_u16 End, f32 T) {
    return ((v2_u16){LerpU16(Start.X, End.X, T), LerpU16(Start.Y, End.Y, T)});
}

static v3_u16 AddV3U16(v3_u16 A, v3_u16 B) { return ((v3_u16){A.X + B.X, A.Y + B.Y, A.Z + B.Z}); }
static v3_u16 SubV3U16(v3_u16 A, v3_u16 B) { return ((v3_u16){A.X - B.X, A.Y - B.Y, A.Z - B.Z}); }
static v3_u16 MulV3U16(v3_u16 A, v3_u16 B) { return ((v3_u16){A.X * B.X, A.Y * B.Y, A.Z * B.Z}); }
static v3_u16 DivV3U16(v3_u16 A, v3_u16 B) { return ((v3_u16){A.X / B.X, A.Y / B.Y, A.Z / B.Z}); }
static v3_u16 ScaleV3U16(v3_u16 V, u16 S) { return ((v3_u16){V.X * S, V.Y * S, V.Z * S}); }
static u16 DotV3U16(v3_u16 A, v3_u16 B) {
    u16 Ret = A.X * B.X + A.Y * B.Y + A.Z * B.Z;
    return Ret;
}
static v3_u16 LerpV3U16(v3_u16 Start, v3_u16 End, f32 T) {
    return ((v3_u16){LerpU16(Start.X, End.X, T), LerpU16(Start.Y, End.Y, T), LerpU16(Start.Z, End.Z, T)});
}
static v3_u16 CrossV3U16(v3_u16 A, v3_u16 B) {
    v3_u16 Ret = (v3_u16){A.Y * B.Z - A.Z * B.Y, A.Z * B.X - A.X * B.Z, A.X * B.Y - A.Y * B.X};
    return Ret;
}

static v4_u16 AddV4U16(v4_u16 A, v4_u16 B) { return ((v4_u16){A.X + B.X, A.Y + B.Y, A.Z + B.Z, A.W + B.W}); }
static v4_u16 SubV4U16(v4_u16 A, v4_u16 B) { return ((v4_u16){A.X - B.X, A.Y - B.Y, A.Z - B.Z, A.W - B.W}); }
static v4_u16 MulV4U16(v4_u16 A, v4_u16 B) { return ((v4_u16){A.X * B.X, A.Y * B.Y, A.Z * B.Z, A.W * B.W}); }
static v4_u16 DivV4U16(v4_u16 A, v4_u16 B) { return ((v4_u16){A.X / B.X, A.Y / B.Y, A.Z / B.Z, A.W / B.W}); }
static v4_u16 ScaleV4U16(v4_u16 V, u16 S) { return ((v4_u16){V.X * S, V.Y * S, V.Z * S, V.W * S}); }
static u16 DotV4U16(v4_u16 A, v4_u16 B) {
    u16 Ret = A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W;
    return Ret;
}
static v4_u16 LerpV4U16(v4_u16 Start, v4_u16 End, f32 T) {
    return ((v4_u16){LerpU16(Start.X, End.X, T), LerpU16(Start.Y, End.Y, T), LerpU16(Start.Z, End.Z, T),
                     LerpU16(Start.W, End.W, T)});
}

static v2_u32 AddV2U32(v2_u32 A, v2_u32 B) { return ((v2_u32){A.X + B.X, A.Y + B.Y}); }
static v2_u32 SubV2U32(v2_u32 A, v2_u32 B) { return ((v2_u32){A.X - B.X, A.Y - B.Y}); }
static v2_u32 MulV2U32(v2_u32 A, v2_u32 B) { return ((v2_u32){A.X * B.X, A.Y * B.Y}); }
static v2_u32 DivV2U32(v2_u32 A, v2_u32 B) { return ((v2_u32){A.X / B.X, A.Y / B.Y}); }
static v2_u32 ScaleV2U32(v2_u32 V, u32 S) { return ((v2_u32){V.X * S, V.Y * S}); }
static u32 DotV2U32(v2_u32 A, v2_u32 B) {
    u32 Ret = A.X * B.X + A.Y * B.Y;
    return Ret;
}
static v2_u32 LerpV2U32(v2_u32 Start, v2_u32 End, f32 T) {
    return ((v2_u32){LerpU32(Start.X, End.X, T), LerpU32(Start.Y, End.Y, T)});
}

static v3_u32 AddV3U32(v3_u32 A, v3_u32 B) { return ((v3_u32){A.X + B.X, A.Y + B.Y, A.Z + B.Z}); }
static v3_u32 SubV3U32(v3_u32 A, v3_u32 B) { return ((v3_u32){A.X - B.X, A.Y - B.Y, A.Z - B.Z}); }
static v3_u32 MulV3U32(v3_u32 A, v3_u32 B) { return ((v3_u32){A.X * B.X, A.Y * B.Y, A.Z * B.Z}); }
static v3_u32 DivV3U32(v3_u32 A, v3_u32 B) { return ((v3_u32){A.X / B.X, A.Y / B.Y, A.Z / B.Z}); }
static v3_u32 ScaleV3U32(v3_u32 V, u32 S) { return ((v3_u32){V.X * S, V.Y * S, V.Z * S}); }
static u32 DotV3U32(v3_u32 A, v3_u32 B) {
    u32 Ret = A.X * B.X + A.Y * B.Y + A.Z * B.Z;
    return Ret;
}
static v3_u32 LerpV3U32(v3_u32 Start, v3_u32 End, f32 T) {
    return ((v3_u32){LerpU32(Start.X, End.X, T), LerpU32(Start.Y, End.Y, T), LerpU32(Start.Z, End.Z, T)});
}
static v3_u32 CrossV3U32(v3_u32 A, v3_u32 B) {
    v3_u32 Ret = (v3_u32){A.Y * B.Z - A.Z * B.Y, A.Z * B.X - A.X * B.Z, A.X * B.Y - A.Y * B.X};
    return Ret;
}

static v4_u32 AddV4U32(v4_u32 A, v4_u32 B) { return ((v4_u32){A.X + B.X, A.Y + B.Y, A.Z + B.Z, A.W + B.W}); }
static v4_u32 SubV4U32(v4_u32 A, v4_u32 B) { return ((v4_u32){A.X - B.X, A.Y - B.Y, A.Z - B.Z, A.W - B.W}); }
static v4_u32 MulV4U32(v4_u32 A, v4_u32 B) { return ((v4_u32){A.X * B.X, A.Y * B.Y, A.Z * B.Z, A.W * B.W}); }
static v4_u32 DivV4U32(v4_u32 A, v4_u32 B) { return ((v4_u32){A.X / B.X, A.Y / B.Y, A.Z / B.Z, A.W / B.W}); }
static v4_u32 ScaleV4U32(v4_u32 V, u32 S) { return ((v4_u32){V.X * S, V.Y * S, V.Z * S, V.W * S}); }
static u32 DotV4U32(v4_u32 A, v4_u32 B) {
    u32 Ret = A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W;
    return Ret;
}
static v4_u32 LerpV4U32(v4_u32 Start, v4_u32 End, f32 T) {
    return ((v4_u32){LerpU32(Start.X, End.X, T), LerpU32(Start.Y, End.Y, T), LerpU32(Start.Z, End.Z, T),
                     LerpU32(Start.W, End.W, T)});
}

static v2_u64 AddV2U64(v2_u64 A, v2_u64 B) { return ((v2_u64){A.X + B.X, A.Y + B.Y}); }
static v2_u64 SubV2U64(v2_u64 A, v2_u64 B) { return ((v2_u64){A.X - B.X, A.Y - B.Y}); }
static v2_u64 MulV2U64(v2_u64 A, v2_u64 B) { return ((v2_u64){A.X * B.X, A.Y * B.Y}); }
static v2_u64 DivV2U64(v2_u64 A, v2_u64 B) { return ((v2_u64){A.X / B.X, A.Y / B.Y}); }
static v2_u64 ScaleV2U64(v2_u64 V, u64 S) { return ((v2_u64){V.X * S, V.Y * S}); }
static u64 DotV2U64(v2_u64 A, v2_u64 B) {
    u64 Ret = A.X * B.X + A.Y * B.Y;
    return Ret;
}
static v2_u64 LerpV2U64(v2_u64 Start, v2_u64 End, f64 T) {
    return ((v2_u64){LerpU64(Start.X, End.X, T), LerpU64(Start.Y, End.Y, T)});
}

static v3_u64 AddV3U64(v3_u64 A, v3_u64 B) { return ((v3_u64){A.X + B.X, A.Y + B.Y, A.Z + B.Z}); }
static v3_u64 SubV3U64(v3_u64 A, v3_u64 B) { return ((v3_u64){A.X - B.X, A.Y - B.Y, A.Z - B.Z}); }
static v3_u64 MulV3U64(v3_u64 A, v3_u64 B) { return ((v3_u64){A.X * B.X, A.Y * B.Y, A.Z * B.Z}); }
static v3_u64 DivV3U64(v3_u64 A, v3_u64 B) { return ((v3_u64){A.X / B.X, A.Y / B.Y, A.Z / B.Z}); }
static v3_u64 ScaleV3U64(v3_u64 V, u64 S) { return ((v3_u64){V.X * S, V.Y * S, V.Z * S}); }
static u64 DotV3U64(v3_u64 A, v3_u64 B) {
    u64 Ret = A.X * B.X + A.Y * B.Y + A.Z * B.Z;
    return Ret;
}
static v3_u64 LerpV3U64(v3_u64 Start, v3_u64 End, f64 T) {
    return ((v3_u64){LerpU64(Start.X, End.X, T), LerpU64(Start.Y, End.Y, T), LerpU64(Start.Z, End.Z, T)});
}
static v3_u64 CrossV3U64(v3_u64 A, v3_u64 B) {
    v3_u64 Ret = (v3_u64){A.Y * B.Z - A.Z * B.Y, A.Z * B.X - A.X * B.Z, A.X * B.Y - A.Y * B.X};
    return Ret;
}

static v4_u64 AddV4U64(v4_u64 A, v4_u64 B) { return ((v4_u64){A.X + B.X, A.Y + B.Y, A.Z + B.Z, A.W + B.W}); }
static v4_u64 SubV4U64(v4_u64 A, v4_u64 B) { return ((v4_u64){A.X - B.X, A.Y - B.Y, A.Z - B.Z, A.W - B.W}); }
static v4_u64 MulV4U64(v4_u64 A, v4_u64 B) { return ((v4_u64){A.X * B.X, A.Y * B.Y, A.Z * B.Z, A.W * B.W}); }
static v4_u64 DivV4U64(v4_u64 A, v4_u64 B) { return ((v4_u64){A.X / B.X, A.Y / B.Y, A.Z / B.Z, A.W / B.W}); }
static v4_u64 ScaleV4U64(v4_u64 V, u64 S) { return ((v4_u64){V.X * S, V.Y * S, V.Z * S, V.W * S}); }
static u64 DotV4U64(v4_u64 A, v4_u64 B) {
    u64 Ret = A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W;
    return Ret;
}
static v4_u64 LerpV4U64(v4_u64 Start, v4_u64 End, f64 T) {
    return ((v4_u64){LerpU64(Start.X, End.X, T), LerpU64(Start.Y, End.Y, T), LerpU64(Start.Z, End.Z, T),
                     LerpU64(Start.W, End.W, T)});
}

/*  =====================================================================================

                        NOTE(acol): Matrix stuff

    ===================================================================================== */

static m2_f32 MulM2F32(m2_f32 A, m2_f32 B) {
    m2_f32 Ret = {0};
    for (i32 i = 0; i < 2; i++) {
        for (i32 j = 0; j < 2; j++) {
            for (i32 k = 0; k < 2; k++) {
                Ret.V[i * 2 + j] += A.V[i * 2 + k] * B.V[k * 2 + j];
            }
        }
    }
    return Ret;
}

static m3_f32 MulM3F32(m3_f32 A, m3_f32 B) {
    m3_f32 Ret = {0};
    for (i32 i = 0; i < 3; i++) {
        for (i32 j = 0; j < 3; j++) {
            for (i32 k = 0; k < 3; k++) {
                Ret.V[i * 3 + j] += A.V[i * 3 + k] * B.V[k * 3 + j];
            }
        }
    }
    return Ret;
}
static m4_f32 MulM4F32(m4_f32 A, m4_f32 B) {
    m4_f32 Ret = {0};
    for (i32 i = 0; i < 4; i++) {
        for (i32 j = 0; j < 4; j++) {
            for (i32 k = 0; k < 4; k++) {
                Ret.V[i * 4 + j] += A.V[i * 4 + k] * B.V[k * 4 + j];
            }
        }
    }
    return Ret;
}
static m2_f64 MulM2F64(m2_f64 A, m2_f64 B) {
    m2_f64 Ret = {0};
    for (i32 i = 0; i < 2; i++) {
        for (i32 j = 0; j < 2; j++) {
            for (i32 k = 0; k < 2; k++) {
                Ret.V[i * 2 + j] += A.V[i * 2 + k] * B.V[k * 2 + j];
            }
        }
    }
    return Ret;
}
static m3_f64 MulM3F64(m3_f64 A, m3_f64 B) {
    m3_f64 Ret = {0};
    for (i32 i = 0; i < 3; i++) {
        for (i32 j = 0; j < 3; j++) {
            for (i32 k = 0; k < 3; k++) {
                Ret.V[i * 3 + j] += A.V[i * 3 + k] * B.V[k * 3 + j];
            }
        }
    }
    return Ret;
}
static m4_f64 MulM4F64(m4_f64 A, m4_f64 B) {
    m4_f64 Ret = {0};
    for (i32 i = 0; i < 4; i++) {
        for (i32 j = 0; j < 4; j++) {
            for (i32 k = 0; k < 4; k++) {
                Ret.V[i * 4 + j] += A.V[i * 4 + k] * B.V[k * 4 + j];
            }
        }
    }
    return Ret;
}
