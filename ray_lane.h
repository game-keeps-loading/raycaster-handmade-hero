#pragma warning(disable : 4505)
#ifndef RAY_LANE_H
#define RAY_LANE_H
#include"ray.h"
#define LANE_WIDTH 8

#if ((LANE_WIDTH == 8))
#if defined(_MSC_VER)
#include "ray_lane_8x.h"
#elif COMPILER_LLVM
#include <xmmintrin.h>
#else#error SSE/NEON Optimization not availible 
#endif

#elif ((LANE_WIDTH == 4))
#if _MSC_VER

#include"ray_lane_4x.h"

#else
#error SSE/NEON Optimization not availible 
#endif

#elif ((LANE_WIDTH == 1 ))

typedef f32 lane_f32;
typedef u32 lane_u32;
typedef v3 lane_v3;

internal void
ConditionalAssign(lane_u32 *Dest, lane_u32 Mask, lane_u32 Source ) {
    Mask = (Mask ? 0xFFFFFFFF : 0);
    *Dest = (((~Mask) & *Dest) | (Mask & Source)); // we want source is assigned if mask is 1 we use positive version of mask to pick source and negative for dest
}

internal void
ConditionalAssign(lane_f32 *Dest, lane_u32 Mask, lane_f32 Source ) {
    ConditionalAssign((lane_u32 *)Dest, Mask, *(lane_u32 *)&Source );
}

internal lane_f32
Max(lane_f32 A, lane_f32 B) {
    return A > B ? A : B;
}

internal b32x 
MaskIsZeroed(lane_u32 LaneMask) {
    return LaneMask == 0;
}

internal u32
HorizontalAdd(lane_u32 A) {
    return A;
}

internal f32
HorizontalAdd(lane_f32 A) {
    return A;
}

internal lane_f32
LaneF32FromLaneU32(lane_u32 A) {
    return (lane_f32)A;
}

internal lane_f32
LaneF32FromU32(u32 A) {
    return (lane_f32)A;
}

internal lane_f32
LaneF32FromF32(f32 A) {
    return (lane_f32)A;
}

internal lane_u32
LaneU32FromU32(u32 A) {
    return (lane_u32)A;
}

// internal lane_u32
// LaneU32FromU32(u32 A, u32 B, u32 C, u32 D) {
//     C = B + D + C;
//     return (lane_u32)(A);
// }

internal lane_v3
LaneV3FromV3(v3 A) {
    return (lane_v3)A;
} 

// other functions
// internal lane_f32
// SquareRoot(lane_f32 A) {
//     return lane_f32(sqrt(A));
// };

inline lane_f32
Square(lane_f32 s) {
    return s*s;
}

//Performs a gather operation
internal lane_f32
GatherF32_(void *BasePtr, u32 Stride, lane_u32 Index) {

    lane_f32 Result =  *(f32 *)((u8 *)BasePtr + Index*Stride);

    return Result;
}

internal lane_v3
operator&(lane_u32 A, lane_v3 B) {
    lane_v3 Result;

    A = (A ? 0xFFFFFFFF : 0);
    u32 x = A & *(u32 *)&B.x;
    u32 y = A & *(u32 *)&B.y;
    u32 z = A & *(u32 *)&B.z;

    Result.x = *(f32 *)&x;
    Result.y = *(f32 *)&y;
    Result.z = *(f32 *)&z;

    return Result;
}

internal lane_u32
LaneU32FromU32(u32 A, u32 B, u32 C, u32 D) {
    lane_u32 Result;
    B = B + C + D;
    Result = A;
    return Result;
}

#else
#error Lane Width must be set to 1 , 4 , 8 or 16:
#endif

#if (LANE_WIDTH != 1)

struct lane_v3 {
    lane_f32 x;
    lane_f32 y;
    lane_f32 z;
};

// addition
internal lane_f32
operator+=(lane_f32 &A, lane_f32 B) {
    A = A + B;
    return A;
}

internal lane_u32
operator+=(lane_u32 &A, lane_u32 B) {
    A = A + B;
    return A;
}

// Addition with addend
internal lane_f32
operator+(lane_f32 A, f32 B) {
    lane_f32 Result;
    Result = A + LaneF32FromF32(B);
    return Result; 
}

internal lane_f32
operator+(f32 B, lane_f32 A) {
    lane_f32 Result;
    Result = A + LaneF32FromF32(B);
    return Result; 
}

internal lane_u32
operator+(lane_u32 A, f32 B) {
    lane_u32 Result;
    Result = A + LaneU32FromU32((u32)B);
    return Result; 
}

internal lane_u32
operator+(f32 B, lane_u32 A) {
    lane_u32 Result;
    Result = A + LaneU32FromU32((u32)B);
    return Result; 
}

// subtraction
internal lane_f32
operator-=(lane_f32 &A, lane_f32 B) {
    A = A - B;
    return A;
}

internal lane_f32
operator-(lane_f32 A, f32 B) {
    lane_f32 Result;
    Result = A - LaneF32FromF32(B);
    return Result; 
}

internal lane_f32
operator-(f32 B, lane_f32 A) {
    lane_f32 Result;
    Result = LaneF32FromF32(B) - A;
    return Result; 
}

// multiplication
internal lane_f32
operator*=(lane_f32 &A, lane_f32 B) {
    A = A * B;
    return A;
}

internal lane_f32
operator*(lane_f32 A, f32 B) {
    lane_f32 Result;
    Result = A * LaneF32FromF32(B);
    return Result; 
}

internal lane_f32
operator*(f32 B, lane_f32 A) {
    lane_f32 Result;
    Result = A * LaneF32FromF32(B);
    return Result; 
}

internal lane_v3
operator*(lane_v3 A, lane_f32 B) {
    lane_v3 Result;

    Result.x = A.x * B;
    Result.y = A.y * B;
    Result.z = A.z * B;

    return Result;
}

internal lane_v3
operator*(lane_f32 A, lane_v3 B) {
    lane_v3 Result;

    Result.x = B.x * A;
    Result.y = B.y * A;
    Result.z = B.z * A;
    return Result;
}

// Divison
internal lane_f32
operator/(lane_f32 A, f32 B) {
    lane_f32 Result;
    Result = A / LaneF32FromF32(B);
    return Result; 
}

internal lane_f32
operator/(f32 B, lane_f32 A) {
    lane_f32 Result;
    Result =  LaneF32FromF32(B) / A;
    return Result; 
}

internal lane_f32
operator/=(lane_f32 &A, lane_f32 B) {
    A = A / B;
    return A;
}

// Bitwise operator
internal lane_f32
operator^=(lane_f32 &A, lane_f32 B) {
    A = A ^ B;
    return A;
}

internal lane_u32
operator^=(lane_u32 &A, lane_u32 B) {
    A = A ^ B;
    return A;
}

internal lane_f32
operator&=(lane_f32 &A, lane_f32 B) {
    A = A & B;
    return A;
}

internal lane_u32
operator&=(lane_u32 &A, lane_u32 B) {
    A = A & B;
    return A;
}

internal lane_v3
operator&(lane_u32 A, lane_v3 B) {
    lane_v3 Result;

    Result.x = A & B.x;
    Result.y = A & B.y;
    Result.z = A & B.z;
    return Result;
}

internal lane_f32
operator|=(lane_f32 &A, lane_f32 B) {
    A = A | B;
    return A;
}

lane_u32 &lane_u32::
operator=(u32 A) {

    *this = LaneU32FromU32(A);
    return *this;
}


lane_f32 &lane_f32::
operator=(f32 A) {
    *this = LaneF32FromF32(A);
    return *this;
}

internal lane_f32
operator-(lane_f32 A) {
    lane_f32 Result =  LaneF32FromF32(0.0f) - A;

    return Result;
}

inline lane_f32
Square(lane_f32 s) {
    return s*s;
}

internal lane_u32
operator>(lane_f32 A, f32 B) {
    lane_u32 Result = (A > LaneF32FromF32(B)); 
    return Result;
}

internal lane_u32
operator>(f32 A, lane_f32 B) {
    lane_u32 Result = (LaneF32FromF32(A) > B); 
    return Result;
}

internal lane_u32
operator<(lane_f32 A, f32 B) {
    lane_u32 Result = (A < LaneF32FromF32(B)); 
    return Result;
}

internal lane_u32
operator<(f32 A, lane_f32 B) {
    lane_u32 Result = (LaneF32FromF32(A) < B); 
    return Result;
}

internal void
ConditionalAssign(lane_u32 *Dest, lane_u32 Mask, lane_u32 Source ) {
    *Dest = (AndNot(Mask,*Dest) | (Mask & Source)); // we want source is assigned if mask is 1 we use positive version of mask to pick source and negative for dest
}

internal lane_f32 
Clamp01(lane_f32 A) {
lane_f32 Result;
Result = Min(Max(A, LaneF32FromF32(0.0f)), LaneF32FromF32(1.0f));
return Result;
}

internal lane_v3
LaneV3FromV3(v3 A) {
    lane_v3 Result;
    Result.x = LaneF32FromF32(A.x);
    Result.y = LaneF32FromF32(A.y);
    Result.z = LaneF32FromF32(A.z);

    return Result;
}

#endif

#define GatherF32(BasePtr, Index, Member) GatherF32_(&(BasePtr)->Member, sizeof(*(BasePtr)), Index)
#define GatherV3(BasePtr, Index, Member) GatherV3_(&(BasePtr)->Member, sizeof(*(BasePtr)), Index)

internal void
ConditionalAssign(lane_v3 *Dest, lane_u32 Mask, lane_v3 Source ) {
    ConditionalAssign(&Dest->x, Mask, Source.x);
    ConditionalAssign(&Dest->y, Mask, Source.y);
    ConditionalAssign(&Dest->z, Mask, Source.z);
}

internal v3
HorizontalAdd(lane_v3 A) {
    v3 Result;

    Result.x = HorizontalAdd(A.x);
    Result.y = HorizontalAdd(A.y);
    Result.z = HorizontalAdd(A.z);

    return Result;
}

internal v3
Extract0(lane_v3 A) {
    v3 Result;
    Result.x = *(f32 *)&A.x;
    Result.y = *(f32 *)&A.y;
    Result.z = *(f32 *)&A.z;

    return Result;
}

internal lane_v3
GatherV3_(void *BasePtr, u32 Stride, lane_u32 Indices) {
    lane_v3 Result;

    Result.x  = GatherF32_((f32 *)BasePtr + 0, Stride, Indices);
    Result.y  = GatherF32_((f32 *)BasePtr + 1, Stride, Indices);
    Result.z  = GatherF32_((f32 *)BasePtr + 2, Stride, Indices);

    return Result;
}   

#endif
