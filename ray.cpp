#define _CRT_SECURE_NO_DEPRECATE // get rid of microsoft warning for fopen
#include "ray.h"
#include "ray_math.h"

// TODO these Struct are here for resolving cicrular dependency

struct random_series {
    lane_u32 State;
};

struct work_order {
    world* World;
    image_u32 Image;
    u32 XMin;
    u32 YMin;
    u32 OnePastXMax;
    u32 OnePastYMax;

    random_series Entropy;
};

struct work_queue {
    u32 WorkOrderCount;
    work_order* WorkOrders;
    u32 RaysPerPixel;
    u32 MaxBounceCount;

    volatile u64 NextWorkOrderIndex;
    volatile u64 BonucesComputed;
    volatile u64 TileRetiredCount;
    volatile u64 LoopsComputed;
};

struct cast_state {
    world* World;
    u32 RaysPerPixel;
    u32 MaxBounceCount;

    f32 FilmX;
    f32 FilmY;
    f32 HalfPixW;
    f32 HalfPixH;

    v3 FilmCenter;
    f32 HalfFilmW;
    f32 HalfFilmH;

    v3 CameraX;
    v3 CameraY;
    v3 CameraZ;
    v3 CameraPosiition;

    random_series Series;
    v3 FinalColor;
    u64 BouncesComputed;
    u64 LoopsComputed;
};

//
internal lane_u32 xorshift32(random_series* Series)
{
    /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
    lane_u32 x = Series->State;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    Series->State = x;

    return x;
}

internal lane_f32
RandomUnilateral(random_series* Series)
{
    lane_f32 Result = LaneF32FromLaneU32(xorshift32(Series) >> 1) / LaneF32FromU32(U32Max >> 1);
    return Result;
}

internal lane_f32
RandomBilateralLane(random_series* Series)
{
    return LaneF32FromF32(-1.0f) + 2.0f * RandomUnilateral(Series);
}

internal u32 GetTotalPixelSize(image_u32 Image)
{
    return sizeof(u32) * Image.Width * Image.Height;
}

internal u32*
GetPixelPointer(image_u32 Image, u32 X, u32 Y)
{
    u32* Result = Image.Pixels + Y * Image.Width + X;

    return Result;
}

internal image_u32 makeImage(u32 Width, u32 Height)
{
    image_u32 Image = {};
    Image.Width = Width;
    Image.Height = Height;
    static u32 outputPixel[MAX_WIDTH * MAX_HEIGHT];
    Image.Pixels = outputPixel;

    return Image;
}

internal void WriteImage(image_u32 Image, const char* OutputFileName)
{
    u32 OutputPixelSize = GetTotalPixelSize(Image);
    bitmap_header Header = {};
    Header.FileType = 0x4D42; // littleendian it is represented as 424D
    Header.FileSize = sizeof(Header) + (OutputPixelSize);
    Header.BitmapOffset = sizeof(Header);
    Header.Size = sizeof(Header) - 14;
    Header.width = Image.Width;
    Header.height = Image.Height;
    Header.Planes = 1;
    Header.BitsPerPixel = 32;
    Header.Compression = 0;
    Header.SizeOfBitmap = OutputPixelSize;
    Header.HorzResolution = 0;
    Header.VertResolution = 0;
    Header.ColorUsed = 0;
    Header.ColorsImportant = 0;

    // const char *OutputFileName = "test.bmp";
    FILE* OutFile = fopen(OutputFileName, "wb");

    if (OutFile) {
        fwrite(&Header, sizeof(Header), 1, OutFile);
        fwrite(Image.Pixels, OutputPixelSize, 1, OutFile); // use the start address of outpixel to write data
        fclose(OutFile);
    } else {
        fprintf(stderr, "[ERROR] Unable to write to file %s. \n", OutputFileName);
    }
}

internal f32
ExactLinearTosRGB(f32 L)
{

    f32 S = L * 12.92f;
    if (L < 0.0f) {
        L = 0.0f;
    } else if (L > 1.0f) {
        L = 1.0f;
    }

    S = L > 0.0031308f ? (f32)(1.055f * pow(L, 1.0f / 2.4f)) - 0.055f : L;

    return S;
}

internal void
CastSampleRays(cast_state* State)
{

    world* World = State->World; // correct
    u32 RaysPerPixel = State->RaysPerPixel; // correct
    u32 MaxBounceCount = State->MaxBounceCount; // correct

    lane_f32 HalfPixW = LaneF32FromF32(State->HalfPixW); // correct
    lane_f32 HalfPixH = LaneF32FromF32(State->HalfPixH); // correct

    lane_f32 FilmX = State->FilmX + HalfPixW; // correct
    lane_f32 FilmY = State->FilmY + HalfPixH; // correct

    lane_v3 FilmCenter = LaneV3FromV3(State->FilmCenter); // correct
    lane_f32 HalfFilmW = LaneF32FromF32(State->HalfFilmW); // correct
    lane_f32 HalfFilmH = LaneF32FromF32(State->HalfFilmH); // correct

    lane_v3 CameraX = LaneV3FromV3(State->CameraX); // correct
    lane_v3 CameraY = LaneV3FromV3(State->CameraY); // correct
    lane_v3 CameraZ = LaneV3FromV3(State->CameraZ); // correct
    lane_v3 CameraPosiition = LaneV3FromV3(State->CameraPosiition); // correct

    random_series Series = State->Series;
    lane_v3 FinalColor = {};

    u64 LoopsComputed = 0;

    // cast_state Result = {};
    u32 LaneWidth = LANE_WIDTH;
    u32 LaneRayCount = RaysPerPixel / LaneWidth;
    assert((LaneRayCount * LaneWidth) == RaysPerPixel);

    f32 Contribution = 1.0f / (f32)(RaysPerPixel);
    lane_u32 BouncesComputed = LaneU32FromU32(0);
    for (u32 RayIndex = 0;
        RayIndex < LaneRayCount;
        ++RayIndex) {
        lane_f32 Offx = FilmX + RandomBilateralLane(&Series) * HalfPixW; // correct
        lane_f32 Offy = FilmY + RandomBilateralLane(&Series) * HalfPixH; // correct
        lane_v3 FilmP = FilmCenter + Offx * HalfFilmW * CameraX + Offy * HalfFilmH * CameraY; // correct
        lane_v3 RayOrigin = CameraPosiition; // correct
        lane_v3 RayDirection = NOZ(FilmP - CameraPosiition); // correct

        lane_v3 Color = {};
        lane_v3 Attenuation = LaneV3FromV3(V3_Initialize(1.0f, 1.0f, 1.0f)); // correct
        lane_f32 MinHitDistance = LaneF32FromF32(0.001f); // correct
        lane_f32 Tolerence = LaneF32FromF32(0.0001f); // correct
        lane_u32 LaneMask = LaneU32FromU32(0xFFFFFFFF); // correct

        // HIT TEST FOR PLANES
        for (u32 BounceCount = 0;
            BounceCount < MaxBounceCount;
            ++BounceCount) {

            lane_f32 HitDistance = LaneF32FromF32(F32Max); // correct
            lane_u32 HitMatIndex = LaneU32FromU32(0); // correct
            lane_u32 LaneIncrement = LaneU32FromU32(1); // correct
            BouncesComputed += (LaneIncrement & LaneMask); // correct
            LoopsComputed += LANE_WIDTH;

            lane_v3 NextOrigin = LaneV3FromV3(V3_Initialize(0.0f, 0.0f, 0.0f));
            lane_v3 NextNormal = LaneV3FromV3(V3_Initialize(0.0f, 0.0f, 0.0f));

            for (u32 PlaneIndex = 0;
                PlaneIndex < World->PlaneCount;
                ++PlaneIndex) {

                plane Plane = World->Planes[PlaneIndex];
                lane_v3 PlaneN = LaneV3FromV3(Plane.N);
                lane_f32 PlaneD = LaneF32FromF32(Plane.d);
        
                lane_f32 denominator = Inner(PlaneN, RayDirection);
                lane_f32 t = (-PlaneD - Inner(PlaneN, RayOrigin)) / denominator;
                
                lane_u32 DenomMask = ((denominator < -Tolerence) | (denominator > Tolerence));

                if (!MaskIsZeroed(DenomMask)) {

                    lane_u32 tMask = ((t > MinHitDistance) & (t < HitDistance));
                    lane_u32 HitMask = (DenomMask & tMask);

                    if (!MaskIsZeroed(HitMask)) {
                        lane_u32 PlaneMatIndex = LaneU32FromU32(Plane.MatIndex);
                        ConditionalAssign(&HitDistance, HitMask, t);
                        ConditionalAssign(&HitMatIndex, HitMask, PlaneMatIndex);
                        ConditionalAssign(&NextNormal, HitMask, PlaneN);
                    }
                }
            }

            for (u32 SphereIndex = 0;
                SphereIndex < World->SphereCount;
                ++SphereIndex) {

                sphere Sphere = World->Spheres[SphereIndex];

                lane_v3 SphereP = LaneV3FromV3(Sphere.P);
                lane_f32 Spherer = LaneF32FromF32(Sphere.r);

                lane_v3 SphereRelativePosition = RayOrigin - SphereP;
                lane_f32 a = Inner(RayDirection, RayDirection);
                lane_f32 b = 2.0f * Inner(SphereRelativePosition, RayDirection);
                lane_f32 c = Inner(SphereRelativePosition, SphereRelativePosition) - Spherer * Spherer;
                
                lane_f32 RootTerm = SquareRoot(b * b - 4.0f * a * c);
                lane_u32 RootMask = (RootTerm > Tolerence);

                if (!MaskIsZeroed(RootMask)) {
                    lane_f32 denominator = 2.0f * a;
                    lane_f32 tp = (-b + RootTerm) / denominator;
                    lane_f32 tn = (-b - RootTerm) / denominator;

                    lane_f32 t = tp;

                    lane_u32 PickMask = ((tn > MinHitDistance) & (tn < tp));

                    ConditionalAssign(&t, PickMask, tn);

                    lane_u32 tMask = (((t > MinHitDistance) & (t < HitDistance)));
                    lane_u32 HitMask = (RootMask & tMask);

                    if (!MaskIsZeroed(HitMask)) {
                        lane_u32 SphereMatIndex = LaneU32FromU32(Sphere.MatIndex);
                        ConditionalAssign(&HitDistance, HitMask, t);
                        ConditionalAssign(&HitMatIndex, HitMask, SphereMatIndex);
                        ConditionalAssign(&NextNormal, HitMask, NOZ(t * RayDirection + SphereRelativePosition));
                    }
                }
            }

            // n way load
            lane_v3 MatEmitColor = LaneMask & (GatherV3(World->Materials, HitMatIndex, EmitColor)); // Must Return 0 on Lane Mask cause it was not hit
            lane_v3 MatRefColor = GatherV3(World->Materials, HitMatIndex, RefColor);
            lane_f32 MatSecular = GatherF32(World->Materials, HitMatIndex, Secular);

            Color += Hadamard(Attenuation, MatEmitColor);
            LaneMask &= (HitMatIndex != LaneU32FromU32(0));

            if (MaskIsZeroed(LaneMask)) {
                break;
            } else {

                lane_f32 CosAtten = (Max(Inner(-RayDirection, NextNormal), LaneF32FromF32(0.0f)));
                Attenuation = Hadamard(Attenuation, CosAtten * MatRefColor);

                RayOrigin += HitDistance * RayDirection;

                lane_v3 PureBounce = RayDirection - 2.0f * Inner(RayDirection, NextNormal) * NextNormal;
                lane_v3 RandomBounce = NOZ(NextNormal + Lane_V3(RandomBilateralLane(&Series), RandomBilateralLane(&Series), RandomBilateralLane(&Series)));
                RayDirection = NOZ(Lerp(RandomBounce, MatSecular, PureBounce));
            }
        }

        FinalColor += Contribution * Color;
    }

    State->BouncesComputed += HorizontalAdd(BouncesComputed);
    State->LoopsComputed += LoopsComputed;
    State->FinalColor = HorizontalAdd(FinalColor);
    State->Series = Series;
}

internal b32x
renderTile(work_queue* Queue)
{

    u64 WorkOrderIndex = LockedADDAndReturnPreviousValue(&Queue->NextWorkOrderIndex, 1);
    if (WorkOrderIndex >= Queue->WorkOrderCount) {
        return false;
    }

    cast_state State;

    work_order* Order = Queue->WorkOrders + WorkOrderIndex;
    State.World = Order->World;

    State.Series = Order->Entropy;

    image_u32 Image = Order->Image;
    u32 XMin = Order->XMin;
    u32 YMin = Order->YMin;
    u32 OnePastXMax = Order->OnePastXMax;
    u32 OnePastYMax = Order->OnePastYMax;

    f32 FilmDist = 1.0f;
    f32 FilmH = Image.Height > Image.Width ? ((f32)Image.Height / (f32)Image.Width) : 1.0f; // Handling Aspect Ratio
    f32 FilmW = Image.Width > Image.Height ? ((f32)Image.Width / (f32)Image.Height) : 1.0f; // Handling Aspect Ratio

    lane_v3 CameraPosiition = V3FromFLoat(0, -10, 1);
    lane_v3 CameraZ = NOZ(CameraPosiition);
    lane_v3 CameraX = NOZ(Cross(V3FromFLoat(0, 0, 1), CameraZ));
    lane_v3 CameraY = NOZ(Cross(CameraZ, CameraX));
    lane_v3 FilmCenter = CameraPosiition - FilmDist * CameraZ;
    State.CameraPosiition = Extract0(CameraPosiition);
    State.CameraX = Extract0(CameraX);
    State.CameraY = Extract0(CameraY);
    State.CameraZ = Extract0(CameraZ);

    State.HalfFilmH = 0.5f * FilmH;
    State.HalfFilmW = 0.5f * FilmW;
    State.FilmCenter = Extract0(FilmCenter);
    State.HalfPixW = 1.0f / (f32)Image.Width;
    State.HalfPixH = 1.0f / (f32)Image.Height;
    State.RaysPerPixel = Queue->RaysPerPixel;
    State.MaxBounceCount = Queue->MaxBounceCount;
    State.BouncesComputed = 0;
    State.LoopsComputed = 0;

    for (u32 y = YMin;
        y < OnePastYMax;
        ++y) {
        u32* Out = GetPixelPointer(Image, XMin, y);
        State.FilmY = -1.0f + 2.0f * ((f32)y / (f32)Image.Height); // Values from [-1,1]
        for (u32 x = XMin;
            x < OnePastXMax;
            ++x) {
            State.FilmX = -1.0f + 2.0f * ((f32)x / (f32)Image.Width); // Values from [-1,1]

            CastSampleRays(&State);

            v3 FinalColor = State.FinalColor;

            f32 r = 255.0f * ExactLinearTosRGB(FinalColor.x);
            f32 g = 255.0f * ExactLinearTosRGB(FinalColor.y);
            f32 b = 255.0f * ExactLinearTosRGB(FinalColor.z);

            u32 BMPValue = ((RoundRealI32ToUInt32(255.0f) << 24) |
                            (RoundRealI32ToUInt32(r) << 16) |
                            (RoundRealI32ToUInt32(g) << 8)  |
                            (RoundRealI32ToUInt32(b) << 0));
            *Out++ = BMPValue;
        }
    }

    LockedADDAndReturnPreviousValue(&Queue->BonucesComputed, State.BouncesComputed);
    LockedADDAndReturnPreviousValue(&Queue->TileRetiredCount, 1);
    LockedADDAndReturnPreviousValue(&Queue->LoopsComputed, State.LoopsComputed);
    return true;
}

int main()
{

    printf("Raycasting....... \n");
    image_u32 Image = makeImage(1280, 720);

    material Materials[7] = {};

    Materials[0].EmitColor = V3_Initialize(0.3f, 0.4f, 0.5f);
    Materials[1].RefColor = V3_Initialize(0.5f, 0.5f, 0.5f);
    Materials[2].RefColor = V3_Initialize(0.7f, 0.5f, 0.3f);
    Materials[3].EmitColor = V3_Initialize(4.0f, 0.0f, 0.0f);
    Materials[4].RefColor = V3_Initialize(0.2f, 0.8f, 0.2f);
    Materials[4].Secular = 0.7f;
    Materials[5].RefColor = V3_Initialize(0.4f, 0.8f, 0.9f);
    Materials[5].Secular = 0.85f;
    Materials[6].RefColor = V3_Initialize(0.95f, 0.95f, 0.95f);
    Materials[6].Secular = 1.0f;

    plane Planes[1] = {};
    Planes[0].MatIndex = 1;
    Planes[0].N = V3_Initialize(0, 0, 1);
    Planes[0].d = 0;

    sphere Sphere[5] = {};
    Sphere[0].MatIndex = 2;
    Sphere[0].P = V3_Initialize(0.0f, 0.0f, 0.0f);
    Sphere[0].r = 1.0f;
    Sphere[1].MatIndex = 3;
    Sphere[1].P = V3_Initialize(3.0f, -2.0f, 0.0f);
    Sphere[1].r = 1.0f;
    Sphere[2].MatIndex = 4;
    Sphere[2].P = V3_Initialize(-2.0f, -1.0f, 2.0f);
    Sphere[2].r = 1.0f;
    Sphere[3].MatIndex = 5;
    Sphere[3].P = V3_Initialize(1.0f, -1.0f, 3.0f);
    Sphere[3].r = 1.0f;
    Sphere[4].MatIndex = 6;
    Sphere[4].P = V3_Initialize(-2.0f, 3.0f, 0.0f);
    Sphere[4].r = 1.0f;

    world World = {};
    World.MaterialCount = ArrayCount(Materials);
    World.Materials = Materials;
    World.PlaneCount = ArrayCount(Planes);
    World.Planes = Planes;
    World.SphereCount = ArrayCount(Sphere);
    World.Spheres = Sphere;

    u32 CoreCount =  GetCPUCount();
    u32 TileWidth = Image.Width / CoreCount;
    u32 TileHeight = TileWidth;
    TileWidth = TileHeight = 64;
    u32 TileCountY = (Image.Height + TileHeight - 1) / TileHeight;
    u32 TileCountX = (Image.Width + TileWidth - 1) / TileWidth;
    u32 TotalTileCount = TileCountX * TileCountY; // correct

    work_queue Queue = {};
    Queue.WorkOrders = (work_order*)malloc(TotalTileCount * sizeof(work_order));
    Queue.MaxBounceCount = 8;
    Queue.RaysPerPixel = 1024;

    printf("Raycaster Configuration %d Cores: with %dx%d and Total Tiles: %d (%dk/tile)tiles %d-Wide Lanes\n",
        CoreCount, TileWidth, TileHeight, TotalTileCount, TileWidth * TileHeight * 1 / 256, LANE_WIDTH); // 4/1024

    printf("Quality is MaxBounces: %d and RaysPerPixel: %d", Queue.MaxBounceCount, Queue.RaysPerPixel);

    for (u32 TileY = 0;
        TileY < TileCountY;
        ++TileY) {
        u32 MinY = TileY * TileHeight;
        u32 OnePastMaxY = MinY + TileHeight;
        if (OnePastMaxY > Image.Height) {
            OnePastMaxY = Image.Height;
        }
        for (u32 TileX = 0;
            TileX < TileCountX;
            ++TileX) {
            u32 MinX = TileX * TileWidth;
            u32 OnePastMaxX = MinX + TileWidth;
            if (OnePastMaxX > Image.Width) {
                OnePastMaxX = Image.Width;
            }

            work_order* Order = Queue.WorkOrders + Queue.WorkOrderCount++;
            assert(Queue.WorkOrderCount <= TotalTileCount);

            Order->World = &World;
            Order->Image = Image;
            Order->XMin = MinX;
            Order->YMin = MinY;
            Order->OnePastXMax = OnePastMaxX;
            Order->OnePastYMax = OnePastMaxY;

            random_series Entropy = { LaneU32FromU32(2397458 + TileX * 29083 + TileY * 97843,
                9878934 + TileX * 89243 + TileY * 12938,
                6783234 + TileX * 29738 + TileY * 97853,
                2945085 + TileX * 54378 + TileY * 89722) };
            Order->Entropy = Entropy;
        }
    }
    assert(Queue.WorkOrderCount == TotalTileCount);

    // memory fencing being done here
    LockedADDAndReturnPreviousValue(&Queue.NextWorkOrderIndex, 0);

    clock_t StartClock = clock();
    for (u32 CoreIndex = 1;
        CoreIndex < CoreCount;
        ++CoreIndex) {
        CreateWorkThread(&Queue);
    }

    printf("\n Tiles Retired %lld \n", Queue.TileRetiredCount);
    while (Queue.TileRetiredCount < TotalTileCount) {
        if (renderTile(&Queue)) {
            printf("Raycasting %lld%% ...", 100 * Queue.TileRetiredCount / (TileCountY * TileCountX));
            fflush(stdout);
        }
    }

    const char* filename = "test.bmp";
    u64 TotalBounce = Queue.LoopsComputed;
    u64 UsedBounces = Queue.BonucesComputed;
    u64 WastedBounces = Queue.LoopsComputed - Queue.BonucesComputed;
    clock_t EndClock = clock();
    clock_t TimeElapsed = EndClock - StartClock;
    printf("\n Raycasting time %ldms \n", TimeElapsed);
    printf("Used Bounces %lld \n", UsedBounces);
    printf("Total Bounces %lld \n", TotalBounce);
    printf("Wasted Bounces %lld (%0.02f%%) \n", WastedBounces, 100.0f * (f32)WastedBounces/(f32)Queue.LoopsComputed);
    printf("Performance %lf ms per bounces \n", (f64)(TimeElapsed) / (f64)Queue.BonucesComputed);

    WriteImage(Image, filename);

    printf("Raycasting Done....... \n");
    return 0;
}

#if (RAY_WIN32 || 1)
#include "win32_ray.cpp"
#else
#error "You don't have the header"
#endif