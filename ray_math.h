#pragma once
#ifndef RAY_MATH_H
#define RAY_MATH_H

#include<math.h>
#include<stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;

typedef s32 b32;
typedef s32 b32x;

#pragma warning(disable : 4201)
union v2 {
  struct {
    f32 x, y;
  };
  f32 e[2];
};
#pragma warning(default : 4201)

inline v2
Perp(v2 v)
{
  v2 Result = { -v.y, v.x };
  return Result;
}

inline v2
V2(f32 x, f32 y)
{
  v2 Result = { x, y };
  return Result;
}

inline v2
V2(s32 x, s32 y)
{
  v2 Result = { (f32)x, (f32)y };
  return Result;
}

inline v2
V2(u32 x, u32 y)
{
  v2 Result = { (f32)x, (f32)y };
  return Result;
}

inline v2
operator+(v2 A, v2 B)
{
  v2 Result;
  Result.x = A.x + B.x;
  Result.y = A.y + B.y;
  return Result;
}

inline v2
operator+=(v2 &A, v2 B)
{
  A = A + B;
  return A;
}

inline v2
operator-(v2 A)
{
  v2 Result;
  Result.x = -A.x;
  Result.y = -A.y;
  return Result;
}

inline v2
operator-(v2 A, v2 B)
{
  v2 Result;
  Result.x = A.x - B.x;
  Result.y = A.y - B.y;
  return Result;
}

inline v2
operator-=(v2 &A, v2 B)
{
  A = A - B;
  return A;
}

inline v2
operator*(f32 A, v2 B)
{
  v2 Result = { A * B.x, A * B.y };
  return Result;
}

inline v2
operator*(v2 B, f32 A)
{
  v2 Result = A * B;
  return Result;
}

inline v2
operator*=(v2 &A, f32 B)
{
  A = B * A;
  return A;
}

inline f32
Inner(v2 A, v2 B)
{
  f32 Result = A.x * B.x + A.y * B.y;
  return Result;
}

inline v2
Hadamard(v2 A, v2 B)
{
  v2 Result = { A.x * B.x, A.y * B.y };
  return Result;
}

inline f32
SquareRoot(f32 s) {
    return (f32)sqrt(s);
}

inline f32
Square(f32 s) {
    return s*s;
}

inline u32
RoundRealI32ToUInt32(f32 F) {
    u32 Result = (u32)(F + 0.5f);
    return Result;
}
inline f32
LengthSq(v2 A)
{
  f32 Result = Inner(A, A);
  return Result;
}

inline f32
Length(v2 A)
{
  f32 Result = SquareRoot(LengthSq(A));
  return Result;
}

inline bool
IsZero(v2 A)
{
  bool Result = A.x == 0 && A.y == 0;
  return Result;
}

//
// v3
//

#pragma warning(disable : 4201)
union v3
{
    struct {
        f32 x, y, z;
    };
    struct {
        f32 u, v, __;
    };
    struct {
        f32 r, g, b;
    };
    struct {
        v2 xy;
        f32 Ignored0__;
    };
    struct {
        f32 Ignored1__;
        v2 yz;
    };
    struct {
        v2 uv;
        f32 Ignored2__;
    };
    struct {
        f32 Ignored3__;
        v2 v__;
    };
    f32 E[3];
};
#pragma warning(default : 4201)

inline v3
V3(f32 x, f32 y, f32 z)
{
  v3 Result = { x, y, z };
  return Result;
}

inline v3
V3(v2 xy, f32 z)
{
  v3 Result = { xy.x, xy.y, z };
  return Result;
}

inline v3
operator+(v3 A, v3 B)
{
  v3 Result;
  Result.x = A.x + B.x;
  Result.y = A.y + B.y;
  Result.z = A.z + B.z;
  return Result;
}

inline v3
operator+=(v3 &A, v3 B)
{
  A = A + B;
  return A;
}

inline v3
operator-(v3 A)
{
  v3 Result;
  Result.x = -A.x;
  Result.y = -A.y;
  Result.z = -A.z;
  return Result;
}

inline v3
operator-(v3 A, v3 B)
{
  v3 Result;
  Result.x = A.x - B.x;
  Result.y = A.y - B.y;
  Result.z = A.z - B.z;
  return Result;
}

inline v3
operator-=(v3 &A, v3 B)
{
  A = A - B;
  return A;
}

inline v3
operator*(f32 A, v3 B)
{
  v3 Result = { A * B.x, A * B.y, A * B.z };
  return Result;
}

inline v3
operator*(v3 B, f32 A)
{
  v3 Result = A * B;
  return Result;
}

inline v3
operator*=(v3 &A, f32 B)
{
  A = B * A;
  return A;
}

inline f32
Inner(v3 A, v3 B) {
  f32 Result = A.x * B.x + A.y * B.y + A.z * B.z;
  return Result;
}

inline v3
Hadamard(v3 A, v3 B) {
  v3 Result = { A.x * B.x, A.y * B.y, A.z * B.z };
  return Result;
}

inline f32
LengthSq(v3 A) {
  f32 Result = Inner(A, A);
  return Result;
}

inline f32
Length(v3 A)
{
  f32 Result = SquareRoot(LengthSq(A));
  return Result;
}

inline bool
IsZero(v3 A)
{
  bool Result = A.x == 0 && A.y == 0 && A.z == 0;
  return Result;
}

inline v3
Normalize(v3 value)
{
  v3 Result = value * (1.0f / Length(value));
  return Result;
}

inline v3
NOZ(v3 A) {
    v3 Result = {};

    f32 LenSq = LengthSq(A);
    if(LenSq > Square(0.0001f))  {
        Result = A * (1.0f / SquareRoot(LenSq));
    }

    return Result;
}

inline v3
Cross(v3 A, v3 B) {
    v3 Result;

    Result.x = A.y*B.z - A.z*B.y;
    Result.y = A.z*B.x - A.x*B.z;
    Result.z = A.x*B.y - A.y*B.x;

    return Result;
}
inline v3
Lerp(v3 A, f32 t, v3 B)
{
  v3 Result = (1 - t) * A + t * B;
  return Result;
}

//
// v4
//

#pragma warning(disable : 4201)
union v4
{
    struct {
        union {
            v3 xyz;
            struct
            {
                f32 x, y, z;
            };
        };
        f32 w;
    };
    struct {
        union {
            v3 rgb;
            struct
            {
                f32 r, g, b;
            };
        };
        f32 a;
    };
    struct
    {
        v2 xy;
        f32 Ignored0_;
        f32 Ignored1_;
    };
    struct
    {
        f32 Ignored2_;
        v2 yz;
        f32 Ignored3_;
    };
    struct
    {
        f32 Ignored4_;
        f32 Ignored5_;
        v2 zw;
    };
    f32 E[4];
};
#pragma warning(default : 4201)

inline v4
V4(f32 x, f32 y, f32 z, f32 w)
{
  v4 Result = { x, y, z, w };
  return Result;
}

inline v4
V4(v3 v, f32 w)
{
  v4 Result = { v.x, v.y, v.z, w };
  return Result;
}

inline v4
operator+(v4 A, v4 B)
{
  v4 Result;
  Result.x = A.x + B.x;
  Result.y = A.y + B.y;
  Result.z = A.z + B.z;
  Result.w = A.w + B.w;
  return Result;
}

inline v4
operator+=(v4 &A, v4 B)
{
  A = A + B;
  return A;
}

inline v4
operator-(v4 A)
{
  v4 Result;
  Result.x = -A.x;
  Result.y = -A.y;
  Result.z = -A.z;
  Result.w = -A.w;
  return Result;
}

inline v4
operator-(v4 A, v4 B)
{
  v4 Result;
  Result.x = A.x - B.x;
  Result.y = A.y - B.y;
  Result.z = A.z - B.z;
  Result.w = A.w - B.w;
  return Result;
}

inline v4
operator-=(v4 &A, v4 B)
{
  A = A - B;
  return A;
}

inline v4
operator*(f32 A, v4 B)
{
  v4 Result = { A * B.x, A * B.y, A * B.z, A * B.w };
  return Result;
}

inline v4
operator*(v4 B, f32 A)
{
  v4 Result = A * B;
  return Result;
}

inline v4
operator*=(v4 &A, f32 B)
{
  A = B * A;
  return A;
}

inline v4
Lerp(v4 A, f32 t, v4 B)
{
  v4 Result = (1 - t) * A + t * B;
  return Result;
}

inline v4
Hadamard(v4 A, v4 B)
{
  v4 Result = { A.x * B.x, A.y * B.y, A.z * B.z, A.w * B.w };
  return Result;
}

//
// misc
//

inline f32
SafeRatioN(f32 numerator, f32 divider, f32 n)
{
  f32 Result = n;

  if(divider != 0) {
    Result = numerator / divider;
  }

  return Result;
}

inline f32
SafeRatio0(f32 numerator, f32 divider)
{
  f32 Result = SafeRatioN(numerator, divider, 0);
  return Result;
}

inline f32
Clamp(f32 v, f32 min, f32 max)
{
  f32 Result = v;
  if(Result < min) {
    Result = min;
  }
  if(Result > max) {
    Result = max;
  }
  return Result;
}

inline f32
Clamp01(f32 v)
{
  f32 Result = Clamp(v, 0, 1);
  return Result;
}

inline f32
Clamp01MapToRange(f32 min, f32 t, f32 max)
{
  f32 Result = 0.0f;
  f32 range = max - min;

  if(range != 0.0f) {
    Result = Clamp01((t - min) / range);
  }

  return Result;
}

inline v2
Clamp01(v2 v)
{
  v2 Result;
  Result.x = Clamp01(v.x);
  Result.y = Clamp01(v.y);
  return Result;
}

inline v3
Clamp01(v3 v)
{
  v3 Result;
  Result.x = Clamp01(v.x);
  Result.y = Clamp01(v.y);
  Result.z = Clamp01(v.z);
  return Result;
}

inline f32
Lerp(f32 A, f32 t, f32 B)
{
  f32 Result = (1 - t) * A + t * B;
  return Result;
}

inline v4
RGBAUnpack4x8(u32 Packed)
{
    v4 Result = {(f32)((Packed >> 0) & 0xFF),
                 (f32)((Packed >> 8) & 0xFF),
                 (f32)((Packed >> 16) & 0xFF),
                 (f32)((Packed >> 24) & 0xFF)};

    return(Result);
}

inline u32
RGBAPack4x8(v4 Unpacked)
{
    u32 Result = ((RoundRealI32ToUInt32(Unpacked.a) << 24) |
                  (RoundRealI32ToUInt32(Unpacked.b) << 16) |
                  (RoundRealI32ToUInt32(Unpacked.g) << 8)  |
                  (RoundRealI32ToUInt32(Unpacked.r) << 0));

    return(Result);
}

inline u32
BGRAPack4x8(v4 Unpacked)
{
    u32 Result = ((RoundRealI32ToUInt32(Unpacked.a) << 24) |
                  (RoundRealI32ToUInt32(Unpacked.r) << 16) |
                  (RoundRealI32ToUInt32(Unpacked.g) << 8)  |
                  (RoundRealI32ToUInt32(Unpacked.b) << 0));

    return(Result);
}
inline b32x
IsInRange(f32 Min, f32 Value, f32 Max)
{
    b32x Result = ((Min <= Value) &&
                   (Value <= Max));

    return(Result);
}

inline v4
Linear1ToSRGB255(v4 C)
{
    v4 Result;

    f32 One255 = 255.0f;

    Result.r = One255*SquareRoot(C.r);
    Result.g = One255*SquareRoot(C.g);
    Result.b = One255*SquareRoot(C.b);
    Result.a = One255*C.a;

    return(Result);
}


#endif