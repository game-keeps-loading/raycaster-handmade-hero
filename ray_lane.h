#ifndef RAY_LANE_H
#define RAY_LANE_H
#include"ray.h"

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

internal void
ConditionalAssign(lane_v3 *Dest, lane_u32 Mask, lane_v3 Source ) {
    ConditionalAssign(&Dest->x, Mask,Source.x);
    ConditionalAssign(&Dest->y, Mask,Source.y);
    ConditionalAssign(&Dest->z, Mask,Source.z);
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

internal v3
HorizontalAdd(lane_v3 A) { 
    v3 Result = {
        HorizontalAdd(A.x),
        HorizontalAdd(A.y),
        HorizontalAdd(A.z)
    };

    return Result;
}
#endif