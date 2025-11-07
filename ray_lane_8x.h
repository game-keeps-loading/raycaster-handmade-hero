#ifndef RAY_LANE_H_8X
#define RAY_LANE_H_8X

#include <intrin.h>
#include<stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef s32 b32;
typedef s32 b32x;

typedef float f32;
typedef double f64;

#define internal static

struct lane_f32 {
    __m256 V;

    lane_f32 &operator=(f32 A);
};

struct lane_u32 {
    __m256i V;

    lane_u32 &operator=(u32 A);
};

// Comparison Operator
internal lane_u32
operator<(lane_f32 A, lane_f32 B) {
    lane_u32 Result;
    Result.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_LT_OQ));
    return Result;
}

// internal lane_u32
// operator<=(lane_f32 A, lane_f32 B) {
//     lane_u32 Result;
//     Result.V = _mm256_castps_si256(_mm_cmple_ps(A.V, B.V));
//     return Result;
// }

internal lane_u32
operator>(lane_f32 A, lane_f32 B) {
    lane_u32 Result;
    Result.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_GT_OQ)); 
    return Result;
}

internal lane_u32
operator>=(lane_f32 A, lane_f32 B) {
    lane_u32 Result;
    Result.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_GE_OQ));
    return Result;
}

internal lane_u32
operator==(lane_f32 A, lane_f32 B) {
    lane_u32 Result;
    Result.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_EQ_OQ));
    return Result;
}

internal lane_u32
operator!=(lane_f32 A, lane_f32 B) {
    lane_u32 Result;
    Result.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_NEQ_OQ));
    return Result;
}

internal lane_u32
operator!=(lane_u32 A, lane_u32 B) {
    lane_u32 Result;    

    // Result.V = _mm_castps_si128(_mm_cmpneq_ps(_mm_castsi128_ps(A.V),_mm_castsi128_ps(B.V)));
     Result.V = _mm256_xor_si256(_mm256_cmpeq_epi32(A.V,B.V),_mm256_set1_epi32 (0xFFFFFFFF));  
    return Result;
}

// Logical Operator
internal lane_f32
operator^(lane_f32 A, lane_f32 B) {
    lane_f32 Result;
    Result.V = _mm256_xor_ps(A.V, B.V);
    return Result;
}

internal lane_u32
operator^(lane_u32 A, lane_u32 B) {
    lane_u32 Result;
    Result.V = _mm256_xor_si256(A.V, B.V);
    return Result;
}

internal lane_f32
operator&(lane_f32 A, lane_f32 B) {
    lane_f32 Result;
    Result.V = _mm256_and_ps(A.V, B.V);
    return Result;
}

internal lane_f32
operator&(lane_u32 A, lane_f32 B) {
    lane_f32 Result;
    Result.V = _mm256_and_ps(_mm256_castsi256_ps(A.V), B.V); 
    return Result;
}

internal lane_u32
operator&(lane_u32 A, lane_u32 B) {
    lane_u32 Result;
    Result.V = _mm256_and_si256(A.V, B.V);
    return Result;
}

internal lane_f32
operator|(lane_f32 A, lane_f32 B) {
    lane_f32 Result;
    Result.V = _mm256_or_ps(A.V, B.V);
    return Result;
}

internal lane_u32
operator|(lane_u32 A, lane_u32 B) {
    lane_u32 Result;
    Result.V = _mm256_or_si256(A.V, B.V);
    return Result;
}

internal lane_u32
AndNot(lane_u32 A, lane_u32 B) {
    lane_u32 Result;
    Result.V = _mm256_andnot_si256(A.V, B.V);
    return Result;
}

// Shifting operations
internal lane_u32
operator<<(lane_u32 A, u32 Shift) {
    lane_u32 Result;
    Result.V = _mm256_slli_epi32(A.V, Shift);
    return Result;
}

internal lane_u32
operator>>(lane_u32 A, u32 Shift) {
    lane_u32 Result;
    Result.V = _mm256_srli_epi32(A.V, Shift);

    return Result;
}

// Arithmatic Operations
internal lane_f32
operator/(lane_f32 A, lane_f32 B) {
    lane_f32 Result;
    Result.V = _mm256_div_ps(A.V, B.V);

    return Result;
}

internal lane_f32
operator+(lane_f32 A, lane_f32 B) {
    lane_f32 Result;
    Result.V = _mm256_add_ps(A.V, B.V);

    return Result;
}

internal lane_u32
operator+(lane_u32 A, lane_u32 B) {
    lane_u32 Result;
    Result.V = _mm256_add_epi32(A.V, B.V);

    return Result;
}


internal lane_f32
operator-(lane_f32 A, lane_f32 B) {
    lane_f32 Result;
    Result.V = _mm256_sub_ps(A.V, B.V);

    return Result;
}

internal lane_f32
operator*(lane_f32 A, lane_f32 B) {
    lane_f32 Result;
    Result.V = _mm256_mul_ps(A.V, B.V);

    return Result;
}

// Conversion operator

internal lane_f32
LaneF32FromLaneU32(lane_u32 A) {
    lane_f32 Result;
    Result.V = _mm256_cvtepi32_ps(A.V);

    return Result;
}

internal lane_f32
LaneF32FromU32(u32 A) {
    lane_f32 Result;
    Result.V = _mm256_set1_ps ((f32)A);

    return Result;
}

internal lane_f32
LaneF32FromF32(f32 A) {
    lane_f32 Result;
    Result.V = _mm256_set1_ps (A);
    return Result;
}

internal lane_u32
LaneU32FromU32(u32 A) {
    lane_u32 Result;
    Result.V = _mm256_set1_epi32 (A);
    return Result;
}

internal lane_u32
LaneU32FromU32(u32 A, u32 B, u32 C, u32 D) {
    lane_u32 Result;
    Result.V = _mm256_setr_epi32(A, B, C, D, A, B, C, D);
    return Result;
}

// other functions
internal lane_f32
SquareRoot(lane_f32 A) {
    lane_f32 Result;
    Result.V = _mm256_sqrt_ps(A.V);

    return Result;
};

// internal lane_f32
// ReverseSquareRoot(lane_f32 A) {
//     lane_f32 Result;
//     Result.V = _mm256_rsqrt_ps(A.V);
// }

// Min and Max

inline lane_f32
Min(lane_f32 A, lane_f32 B) {
    lane_f32 Result;
    
    Result.V = _mm256_min_ps(A.V,B.V);

    return Result;
}

inline lane_f32
Max(lane_f32 A, lane_f32 B) {
    lane_f32 Result;
    
    Result.V = _mm256_max_ps(A.V,B.V);

    return Result;
}

internal void
ConditionalAssign(lane_f32 *Dest, lane_u32 Mask, lane_f32 Source ) {
    __m256 MaskPs = _mm256_castsi256_ps(Mask.V); 
    Dest->V = _mm256_or_ps(_mm256_andnot_ps(MaskPs,Dest->V),
                           _mm256_and_ps(MaskPs,Source.V)); // we want source is assigned if mask is 1 we use positive version of mask to pick source and negative for dest
}

//Performs a gather operation
internal lane_f32
GatherF32_(void *BasePtr, u32 Stride, lane_u32 Indices) {
    u32 *V = (u32 *)&Indices.V;

    lane_f32 Result;
    Result.V = _mm256_setr_ps(*(f32 *)((u8 *)BasePtr + V[0]*Stride),
                              *(f32 *)((u8 *)BasePtr + V[1]*Stride),
                              *(f32 *)((u8 *)BasePtr + V[2]*Stride),
                              *(f32 *)((u8 *)BasePtr + V[3]*Stride),
                              *(f32 *)((u8 *)BasePtr + V[4]*Stride),
                              *(f32 *)((u8 *)BasePtr + V[5]*Stride),
                              *(f32 *)((u8 *)BasePtr + V[6]*Stride),
                              *(f32 *)((u8 *)BasePtr + V[7]*Stride));

    return Result;
}

internal b32x
MaskIsZeroed(lane_u32 A) {
    return _mm256_movemask_epi8(A.V) == 0;
}

// internal u64
// HorizontalAdd(lane_u32 A) {
//     u32 *V = (u32 *)&A.V; // cast the value to a u32 Pointer so we can offset it to get the values
//     u64 Result = (u64)V[0] + (u64)V[1] + (u64)V[2];

//     return Result;
// }

// internal f32
// HorizontalAdd(lane_f32 A) {
//     f32 *V = (f32 *)&A.V; 
//     f32 Result = V[0] + V[1] + V[2];

//     return Result;
// }

internal u64
HorizontalAdd(lane_u32 A) {
    u32 *V = (u32 *)&A.V;
    u64 Result = 0;
    for (u32 i = 0; i < 8; ++i) {
        Result += (u64)V[i];
    }
    return Result;
}

internal f32
HorizontalAdd(lane_f32 A) {
    f32 *V = (f32 *)&A.V;
    f32 Result = 0.0f;
    for (u32 i = 0; i < 8; ++i) {
        Result += V[i];
    }
    return Result;
}

#endif