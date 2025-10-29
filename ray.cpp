#pragma once
#define _CRT_SECURE_NO_DEPRECATE // get rid of microsoft warning for fopen
#include"ray.h"

internal f32
RandomUnilateral(random_series *State) {
    return ((f32)xorshift32(State)/ (f32)U32Max);
}

internal f32
RandomBilateral(random_series *State) {
    return -1.0f + 2.0f*RandomUnilateral(State);
}


internal u32 GetTotalPixelSize(image_u32 Image) {
    return sizeof(u32)*Image.Width*Image.Height;
}

internal u32 *
GetPixelPointer(image_u32 Image, u32 X, u32 Y) {
    u32 *Result = Image.Pixels  + Y*Image.Width + X ;

    return  Result;
}

internal image_u32 makeImage(u32 Width, u32 Height) {
    image_u32 Image = {};
    Image.Width = Width;
    Image.Height = Height;
    static u32 outputPixel[MAX_WIDTH*MAX_HEIGHT];
    Image.Pixels = outputPixel;

    return Image;
}

internal void WriteImage(image_u32 Image, const char *OutputFileName) {
    u32 OutputPixelSize = GetTotalPixelSize(Image);
    bitmap_header Header = {};
    Header.FileType = 0x4D42; // littleendian it is represented as 424D 
    Header.FileSize = sizeof(Header)+(OutputPixelSize);
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
    FILE *OutFile = fopen(OutputFileName, "wb");

    if(OutFile) {
        fwrite(&Header, sizeof(Header), 1, OutFile);
        fwrite(Image.Pixels, OutputPixelSize, 1, OutFile); // use the start address of outpixel to write data
        fclose(OutFile);
    } else {
        fprintf(stderr, "[ERROR] Unable to write to file %s. \n", OutputFileName);
    }
}

internal f32
ExactLinearTosRGB(f32 L) {

    f32 S = L*12.92f;
    if(L < 0.0f) {
        L = 0.0f;
    } else if (L > 1.0f) {
        L = 1.0f;
    }

    S = L > 0.0031308f ? (f32)(1.055f*pow(L, 1.0f/2.4f)) - 0.055f : L;

    return S;
}

struct cast_result {
    v3 FinalColor;
    u64 BouncesComputed;
};

internal cast_result 
CastSampleRays(world *World, u32 RaysPerPixel, u32 MaxBounceCount,
                f32 FilmX, f32 FilmY, f32 HalfPixW, f32 HalfPixH,
                v3 FilmCenter, f32 HalfFilmW, f32 HalfFilmH,
                v3 CameraX, v3 CameraY, v3 CameraPosiition,
                random_series State) {
    cast_result Result = {};
    f32 Contribution = 1.0f/(f32)RaysPerPixel;

    for(u32 RayIndex = 0;
        RayIndex <RaysPerPixel;
        ++RayIndex) {
                    f32 Offx = FilmX + RandomBilateral(&State)*HalfPixW;
                    f32 Offy = FilmY + RandomBilateral(&State)*HalfPixH; 
                    v3 FilmP =  FilmCenter + Offx*HalfFilmW*CameraX + Offy*HalfFilmH*CameraY;
                    v3 RayOrigin = CameraPosiition;
                    v3 RayDirection = NOZ(FilmP - CameraPosiition);
            
            v3 Color = {};
            v3 Attenuation = V3(1.0f, 1.0f, 1.0f);;

            f32 MinHitDistance = 0.001f;
            f32 Tolerence = 0.0001f;
            // HIT TEST FOR PLANES
            for(u32 BounceCount = 0;
                BounceCount < MaxBounceCount;
                ++BounceCount){    
                    ++Result.BouncesComputed;
                    f32 HitDistance = F32Max;
                
                    u32 HitmMatIndex = 0;
                    v3 NextOrigin = V3(0.0f, 0.0f, 0.0f);
                    v3 NextNormal = V3(0.0f, 0.0f, 0.0f);
                
                    ++Result.BouncesComputed;
                
                    for(u32 PlaneIndex = 0;
                    PlaneIndex < World->PlaneCount;
                    ++PlaneIndex) {
                    
                        plane Plane = World->Planes[PlaneIndex];
                    
                        f32 denominator = Inner(Plane.N,RayDirection);
                        if (( denominator < -Tolerence) ||  (denominator > Tolerence)) {
                        f32 t = (-Plane.d - Inner(Plane.N, RayOrigin))/denominator;
                        
                        if((t > MinHitDistance && (t < HitDistance))) {
                            HitDistance = t;
                            HitmMatIndex = Plane.MatIndex;
                        
                            NextOrigin = RayOrigin + t*RayDirection;
                            NextNormal = Plane.N;
                        }
                    }
                }
            
                for(u32 SphereIndex = 0;
                    SphereIndex < World->SphereCount;
                    ++SphereIndex) {
                    
                        sphere Sphere = World->Spheres[SphereIndex];
                    
                        v3 SphereRelativePosition = RayOrigin - Sphere.P;
                        f32 a = Inner(RayDirection,RayDirection);
                        f32 b = 2.0f*Inner(SphereRelativePosition,RayDirection);
                        f32 c = Inner(SphereRelativePosition, SphereRelativePosition) - Sphere.r*Sphere.r;
                    
                        f32 denominator = 2.0f*a;
                        f32 RootTerm = SquareRoot(b*b - 4.0f*a*c);
                    
                        if (RootTerm > Tolerence) {
                        
                        f32 tp = (-b + RootTerm) / denominator;
                        f32 tn = (-b - RootTerm) / denominator;
                        
                        f32 t = tp;
                        
                        if ((tn > MinHitDistance) && (tn < tp)) {
                            t = tn;
                        }
                    
                        if(((t > MinHitDistance) && (t < HitDistance))) {
                            HitDistance = t;
                            HitmMatIndex = Sphere.MatIndex;

                            // NextOrigin += HitDistance * RayDirection;
                            NextOrigin = RayOrigin - t*RayDirection;
                            NextNormal = NOZ(t*RayDirection + SphereRelativePosition);
                        }
                    }
                }
                
                
                if(HitmMatIndex) {
                    material Mat = World->Materials[HitmMatIndex];
                
                    Color += Hadamard(Attenuation,Mat.EmitColor);
                
                    f32 CosAtten = Inner(-RayDirection, NextNormal);
                    CosAtten = CosAtten < 0 ? 0 : CosAtten;

                    Attenuation = Hadamard(Attenuation, CosAtten*Mat.RefColor);
                    RayOrigin += HitDistance*RayDirection;
                    v3 PureBounce = RayDirection - 2.0f*Inner(RayDirection,NextNormal)*NextNormal;
                    v3 RandomBounce = NOZ(NextNormal + V3(RandomBilateral(&State),
                                      RandomBilateral(&State), 
                                      RandomBilateral(&State)));
                    RayDirection = NOZ(Lerp(RandomBounce, Mat.Scatter, PureBounce));
                } else {
                    material Mat = World->Materials[HitmMatIndex];
                    Color += Hadamard(Attenuation,Mat.EmitColor);
                    break;
                }
            }

            Result.FinalColor += Contribution*Color;
        }
        return Result;
}

internal b32x
renderTile(work_queue *Queue) {

    u64 WorkOrderIndex = LockedADDAndReturnPreviousValue(&Queue->NextWorkOrderIndex,1);
    if (WorkOrderIndex >= Queue->WorkOrderCount) {
        return false;
    }

    work_order *Order = Queue->WorkOrders + WorkOrderIndex;
    world *World =  Order->World;

    random_series State = Order->Entropy;

    image_u32 Image = Order->Image;
    u32 XMin = Order->XMin;
    u32 YMin = Order->YMin;
    u32 OnePastXMax = Order->OnePastXMax;
    u32 OnePastYMax = Order->OnePastYMax;
    v3 CameraPosiition = V3(0, -10, 1);
    v3 CameraZ  = NOZ(CameraPosiition);
    v3 CameraX = NOZ(Cross(V3(0,0,1),CameraZ));
    v3 CameraY = NOZ(Cross(CameraZ,CameraX));

    f32 FilmDist = 1.0f;
    f32 FilmH =  Image.Height > Image.Width ? ((f32)Image.Height/(f32)Image.Width) : 1.0f; // Handling Aspect Ratio
    f32 FilmW =  Image.Width > Image.Height ? ((f32)Image.Width/(f32)Image.Height) : 1.0f; // Handling Aspect Ratio
    f32 HalfFilmH = 0.5f*FilmH;
    f32 HalfFilmW = 0.5f*FilmW;
    v3 FilmCenter = CameraPosiition - FilmDist * CameraZ;
    f32 HalfPixW = 1.0f / (f32) Image.Width;
    f32 HalfPixH = 1.0f / (f32) Image.Height;
    u32 RaysPerPixel = Queue->RaysPerPixel;
    u32 MaxBounceCount = Queue->MaxBounceCount;
    u64 BouncesComputed = 0;

    for(u32 y = YMin;
        y < OnePastYMax;
        ++y) {
        u32 *Out = GetPixelPointer(Image, XMin, y);
        f32 FilmY = -1.0f + 2.0f*((f32)y/(f32)Image.Height); // Values from [-1,1]
        for(u32 x = XMin;
            x < OnePastXMax;
            ++x) {
            f32 FilmX = -1.0f + 2.0f*((f32)x/(f32)Image.Width); // Values from [-1,1]
            
            cast_result Result = CastSampleRays(World, RaysPerPixel, MaxBounceCount,
                FilmX, FilmY, HalfPixW, HalfPixH,
                FilmCenter, HalfFilmW, HalfFilmH, 
                CameraX, CameraY, CameraPosiition,
                State);

                BouncesComputed += Result.BouncesComputed;
                v3 FinalColor = Result.FinalColor;
        v4 BMPColor = {
            255.0f*ExactLinearTosRGB(FinalColor.r),
            255.0f*ExactLinearTosRGB(FinalColor.g),
            255.0f*ExactLinearTosRGB(FinalColor.b),
            255.0f
        };
        u32 BMPValue = BGRAPack4x8(BMPColor);
        *Out++ = BMPValue;
    }
}

        LockedADDAndReturnPreviousValue(&Queue->BonucesComputed, BouncesComputed);
        LockedADDAndReturnPreviousValue(&Queue->TileRetiredCount, 1 );
        return true;
    }



int main() {

    printf("Raycasting....... \n");
    image_u32 Image = makeImage(1280, 720);

    material Materials[7] = {};

    Materials[0].EmitColor = V3(0.3f, 0.4f, 0.5f);
    Materials[1].RefColor = V3(0.5f, 0.5f, 0.5f);
    Materials[2].RefColor = V3(0.7f, 0.5f, 0.3f);
    Materials[3].EmitColor = V3(7.0f, 0.0f, 0.0f);
    Materials[4].RefColor = V3(0.2f, 0.8f, 0.2f);
    Materials[4].Scatter = 0.7f;
    Materials[5].RefColor = V3(0.4f, 0.8f, 0.9f);
    Materials[5].Scatter = 0.85f;
    Materials[6].RefColor = V3(0.95f, 0.95f, 0.95f);
    Materials[6].Scatter = 1.0f;

    plane Planes[1] = {};
    Planes[0].MatIndex = 1;
    Planes[0].N = V3(0, 0, 1);
    Planes[0].d = 0;

    sphere Sphere[5] = {};
    Sphere[0].MatIndex = 2;
    Sphere[0].P = V3(0.0f, 0.0f, 0.0f);
    Sphere[0].r = 1.0f;
    Sphere[1].MatIndex = 3;
    Sphere[1].P = V3(3.0f, -2.0f, 0.0f);
    Sphere[1].r = 1.0f;
    Sphere[2].MatIndex = 4;
    Sphere[2].P = V3(-2.0f, -1.0f, 2.0f);
    Sphere[2].r = 1.0f;
    Sphere[3].MatIndex = 5;
    Sphere[3].P = V3(1.0f, -1.0f, 3.0f);
    Sphere[3].r = 1.0f;
    Sphere[4].MatIndex = 6;
    Sphere[4].P = V3(-2.0f, 3.0f, 0.0f);
    Sphere[4].r = 1.0f;

    world World = {};
    World.MaterialCount = ArrayCount(Materials);
    World.Materials = Materials;
    World.PlaneCount = ArrayCount(Planes);
    World.Planes = Planes;
    World.SphereCount = ArrayCount(Sphere);
    World.Spheres = Sphere;

    u32 CoreCount = GetCPUCount();
    // u32 CoreCount = 8;
    u32 TileWidth = Image.Width / CoreCount;
    u32 TileHeight = TileWidth;
    TileWidth = TileHeight = 64;
    u32 TileCountY = (Image.Height + TileHeight - 1)/ TileHeight;
    u32 TileCountX = (Image.Width + TileWidth - 1)/ TileWidth;
    u32 TotalTileCount = TileCountX*TileCountY; // correct


    work_queue Queue = {};
    Queue.WorkOrders = (work_order *)malloc(TotalTileCount*sizeof(work_order));
    Queue.MaxBounceCount = 8;
    Queue.RaysPerPixel = 1024;

    printf("Raycaster Configuration %d Cores: with %dx%d and Total Tiles: %d (%dk/tile)tiles \n",
        CoreCount, TileWidth, TileHeight, TotalTileCount , TileWidth*TileHeight*1/256); // 4/1024 
    
    printf("Quality is MaxBounces: %d and RaysPerPixel: %d",Queue.MaxBounceCount, Queue.RaysPerPixel);

    for(u32 TileY = 0;
        TileY < TileCountY;
        ++TileY) {
            u32 MinY = TileY * TileHeight;
            u32 OnePastMaxY = MinY + TileHeight;
            if(OnePastMaxY > Image.Height) { OnePastMaxY = Image.Height; }
            for(u32 TileX = 0;
                TileX < TileCountX;
                ++TileX) {
                    u32 MinX = TileX * TileWidth;
                    u32 OnePastMaxX = MinX + TileWidth;
                    if(OnePastMaxX > Image.Width) { OnePastMaxX = Image.Width; }

                    work_order *Order = Queue.WorkOrders + Queue.WorkOrderCount++;
                    assert(Queue.WorkOrderCount <= TotalTileCount);

                    Order->World = &World;
                    Order->Image = Image;
                    Order->XMin = MinX;
                    Order->YMin = MinY;
                    Order->OnePastXMax = OnePastMaxX;
                    Order->OnePastYMax = OnePastMaxY;

                    random_series Entropy = { TileX*1024 + TileY*5120 };
                    Order->Entropy = Entropy;
                }
            }

        assert(Queue.WorkOrderCount == TotalTileCount);

        // memory fencing being done here
        LockedADDAndReturnPreviousValue(&Queue.NextWorkOrderIndex, 0);

        clock_t StartClock = clock();
        for(u32 CoreIndex = 1;
            CoreIndex < CoreCount;
            ++CoreIndex) {
                CreateWorkThread(&Queue);
            }

        printf("\n Tiles Retired %lld \n", Queue.TileRetiredCount);
        while (Queue.TileRetiredCount < TotalTileCount) {
            if(renderTile(&Queue))
            {
                printf("Raycasting %lld%% ...",100 * Queue.TileRetiredCount / (TileCountY * TileCountX));
                fflush(stdout);
            }
        }

    const char *filename = "test.bmp";

    clock_t EndClock = clock();
    clock_t TimeElapsed = EndClock - StartClock;
    printf("\n Raycasting time %ldms \n", TimeElapsed);
    printf("Total Bounces %lld \n", Queue.BonucesComputed);
    printf("Performance %lf ms per bounces \n", (f64)(TimeElapsed)/(f64)Queue.BonucesComputed);
    
    WriteImage(Image, filename);
    
    printf("Raycasting Done....... \n");
    return 0;
}

#if RAY_WIN32
#include"win32_ray.cpp"
#else
#error "You don't have the header"
#endif