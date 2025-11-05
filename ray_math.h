#ifndef RAY_MATH_H
#define RAY_MATH_H

#include<math.h>
#include<stdint.h>
#include"ray_lane.h"
#include"ray.h"

// typedef uint8_t u8;
// typedef uint16_t u16;
// typedef uint32_t u32;
// typedef uint64_t u64;

// typedef int8_t s8;
// typedef int16_t s16;
// typedef int32_t s32;
// typedef int64_t s64;

// typedef float f32;

// typedef s32 b32;
// typedef s32 b32x;

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
Square_Scaler(f32 s) {
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

inline v3
V3_Initialize(f32 x, f32 y, f32 z)
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

inline lane_v3
V3FromFLoat(f32 x, f32 y, f32 z) {
  lane_v3 Result;
  Result.x = LaneF32FromF32(x);
  Result.y = LaneF32FromF32(y);
  Result.z = LaneF32FromF32(z);

  return Result;
}

inline lane_v3
operator+(lane_v3 A, lane_v3 B)
{
  lane_v3 Result;
  Result.x = A.x + B.x;
  Result.y = A.y + B.y;
  Result.z = A.z + B.z;
  return Result;
}

inline lane_v3
operator+=(lane_v3 &A, lane_v3 B)
{
  A = A + B;
  return A;
}

inline lane_v3
operator-(lane_v3 A)
{
  lane_v3 Result;
  Result.x = -A.x;
  Result.y = -A.y;
  Result.z = -A.z;
  return Result;
}

inline lane_v3
operator-(lane_v3 A, lane_v3 B)
{
  lane_v3 Result;
  Result.x = A.x - B.x;
  Result.y = A.y - B.y;
  Result.z = A.z - B.z;
  return Result;
}

inline lane_v3
operator-=(lane_v3 &A, lane_v3 B)
{
  A = A - B;
  return A;
}

inline lane_v3
operator*(f32 A, lane_v3 B)
{
  lane_v3 Result = { A * B.x, A * B.y, A * B.z };
  return Result;
}

inline lane_v3
operator*(lane_v3 B, f32 A)
{
  lane_v3 Result = A * B;
  return Result;
}

inline lane_v3
operator*=(lane_v3 &A, f32 B)
{
  A = B * A;
  return A;
}

inline lane_f32
Inner(lane_v3 A, lane_v3 B) {
  lane_f32 Result = A.x * B.x + A.y * B.y + A.z * B.z;
  return Result;
}

inline lane_v3
Hadamard(lane_v3 A, lane_v3 B) {
  lane_v3 Result = { A.x * B.x, A.y * B.y, A.z * B.z };
  return Result;
}

inline lane_f32
LengthSq(lane_v3 A) {
  lane_f32 Result = Inner(A, A);
  return Result;
}

inline lane_f32
Length(lane_v3 A)
{
  lane_f32 Result = SquareRoot(LengthSq(A));
  return Result;
}

// inline bool
// IsZero(lane_v3 A)
// {
//   bool Result = A.x == LaneF32FromF32(0.0f) & A.y == LaneF32FromF32(0.0f)  & A.z == LaneF32FromF32(0.0f);
//   return Result;
// }

inline lane_v3
Normalize(lane_v3 value)
{
  lane_v3 Result = value * (1.0f / Length(value));
  return Result;
}

// possible issue
inline lane_v3
NOZ(lane_v3 A) {
    lane_v3 Result = {};

    lane_f32 LenSq = LengthSq(A);
    lane_u32 Mask =  (LenSq > Square_Scaler(0.0001f));
        ConditionalAssign(&Result, Mask, A * (1.0f / SquareRoot(LenSq)));

    return Result;
}

inline lane_v3
Cross(lane_v3 A, lane_v3 B) {
    lane_v3 Result;

    Result.x = A.y*B.z - A.z*B.y;
    Result.y = A.z*B.x - A.x*B.z;
    Result.z = A.x*B.y - A.y*B.x;

    return Result;
}

inline lane_v3
Lerp(lane_v3 A, lane_f32 t, lane_v3 B)
{
  lane_v3 Result = (1.0f - t) * A + t * B;
  return Result;
}

internal lane_v3
Lane_V3(lane_f32 x, lane_f32 y, lane_f32 z) {
    lane_v3 Result;

    Result.x = x;
    Result.y = y;
    Result.z = z;

    return Result;
}

#endif