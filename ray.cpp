#include<stdio.h>
#include<stdlib.h>
#include"ray_math.h"
#include<float.h>
#include<math.h>

typedef char unsigned u8;
typedef short unsigned u16;
typedef int unsigned u32;

typedef char s8;
typedef short s16;
typedef int s32;

typedef s32 b32;
typedef s32 b32x;

typedef float f32;

#define F32Max FLT_MAX
#define F32Min FLT_MIN

#define Pi32 3.14159265359f
#define Tau32 6.2831853071f

#define internal static
#define MAX_WIDTH 3840
#define MAX_HEIGHT 2160

//u -> unsigned 
//s -> signed
#pragma pack(push, 2)
struct bitmap_header {
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    s32 width;
    s32 height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 SizeOfBitmap;
    s32 HorzResolution;
    s32 VertResolution;
    u32 ColorUsed;
    u32 ColorsImportant;
};
#pragma pack(pop)

struct  image_u32 {
    u32 Width;
    u32 Height;
    u32 *Pixels;
};

struct  material {
    f32 Scatter; // 0 is pure diffuse 1 is mirror
    v3 EmitColor;
    v3 RefColor;
};

struct plane {
    v3 N; //vector
    f32 d;
    u32 MatIndex;
};

struct sphere {
    v3 P; //vector
    f32 r;
    u32 MatIndex;
};

struct world {
    u32 MaterialCount;
    material *Materials;

    u32 PlaneCount;
    plane *Planes;

    u32 SphereCount;
    sphere *Spheres;
};

internal f32
RandomUnilateral(void) {
    return ((f32)rand()/ (f32)RAND_MAX);
}

internal f32
RandomBilateral(void) {
    return -1.0f + 2.0f*RandomUnilateral();
}

internal v3
RayCast (world *World, v3 RayOrigin, v3 RayDirection) {

    v3 Result = {};
    v3 Attenuation = V3(1.0f, 1.0f, 1.0f);;

    f32 MinHitDistance = 0.001f;
    f32 Tolerence = 0.0001f;

    // HIT TEST FOR PLANES
    for(u32 RayCount = 0;
        RayCount < 8;
        ++RayCount){    

            f32 HitDistance = F32Max;

            u32 HitmMatIndex = 0;
            v3 NextOrigin = V3(0.0f, 0.0f, 0.0f);
            v3 NextNormal = V3(0.0f, 0.0f, 0.0f);;

            for(u32 PlaneIndex = 0;
            PlaneIndex < World->PlaneCount;
            ++PlaneIndex) {

                plane Plane = World->Planes[PlaneIndex];

                f32 denominator = Inner(Plane.N,RayDirection);
                if (( denominator < -Tolerence) >  (denominator >Tolerence)) {
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


            Result += Hadamard(Attenuation,Mat.EmitColor);
            Attenuation = Hadamard(Attenuation, Mat.RefColor);

            RayOrigin = NextOrigin;

            // Reflection

            v3 PureBounce = RayDirection - 2.0f*Inner(RayDirection,NextNormal)*NextNormal;
            v3 RandomBounce = NOZ(NextNormal + V3(RandomBilateral(), RandomBilateral(), RandomBilateral()));
            RayDirection = NOZ(Lerp(RandomBounce, Mat.Scatter, PureBounce));
        } else {
            material Mat = World->Materials[HitmMatIndex];
            Result += Hadamard(Attenuation,Mat.EmitColor);
            break;
        }
    }
    return Result;
}

internal u32 GetTotalPixelSize(image_u32 Image) {
    return sizeof(u32)*Image.Width*Image.Height;
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

int main() {

    printf("Raycasting....... \n");

    image_u32 Image = makeImage(2560, 1440);

    material Materials[3] = {};

    Materials[0].EmitColor = V3(0.3f, 0.4f, 0.5f);
    Materials[1].RefColor = V3(0.5f, 0.5f, 0.5f);
    Materials[2].RefColor = V3(0.7f, 0.5f, 0.3f);

    plane Planes = {};
    Planes.MatIndex = 1;
    Planes.N = V3(0, 0, 1);
    Planes.d = 0;

    sphere Sphere = {};
    Sphere.MatIndex = 2;
    Sphere.P = V3(0.0f, 0.0f, 0.0f);
    Sphere.r = 1.0f;

    world World = {};
    World.MaterialCount = 3;
    World.Materials = Materials;
    World.PlaneCount = 1;
    World.Planes = &Planes;
    World.SphereCount = 1;
    World.Spheres = &Sphere;

    v3 CameraPosiition = V3(0, -10, 1);
    v3 CamerZ  = NOZ(CameraPosiition);
    v3 CameraX = NOZ(Cross(V3(0,0,1),CamerZ));
    v3 CameraY = NOZ(Cross(CamerZ,CameraX));

    f32 FilmDist = 1.0f;
    f32 FilmH =  Image.Height > Image.Width ? ((f32)Image.Height/(f32)Image.Width) : 1.0f; // Handling Aspect Ratio
    f32 FilmW =  Image.Width > Image.Height ? ((f32)Image.Width/(f32)Image.Height) : 1.0f; // Handling Aspect Ratio
    f32 HalfFilmH = 0.5f*FilmH;
    f32 HalfFilmW = 0.5f*FilmW;
    v3 FilmCenter = CameraPosiition - FilmDist*CamerZ;

    u32 *Out = Image.Pixels;
    u32 RaysPerPixel = 16;
    for(u32 y = 0; y<Image.Height; ++y) {
        f32 FilmY = -1.0f + 2.0f*((f32)y/(f32)Image.Height); // Values from [-1,1]
        for(u32 x=0; x<Image.Width; ++x) {
            f32 FilmX = -1.0f + 2.0f*((f32)x/(f32)Image.Width); // Values from [-1,1]
            v3 FilmP =  FilmCenter + FilmX*HalfFilmW*CameraX + FilmY*HalfFilmH*CameraY;
            v3 RayOrigin = CameraPosiition;
            v3 RayDirection = NOZ(FilmP - CameraPosiition);
            
            v3 Color = {};
            f32 Contribution = 1.0f/(f32)RaysPerPixel;
            for(u32 RayIndex = 0;
                RayIndex <RaysPerPixel;
                ++RayIndex) {
            Color += Contribution*RayCast(&World, RayOrigin, RayDirection);
            }

            v4 BMPColor = V4(255.0f*Color, 255.0f);
            u32 BMPValue = BGRAPack4x8(BMPColor);
            *Out++ = BMPValue;
        }
        if ((y % 128) == 0) {
            printf("Raycasting Rows %d%%  \n",100* y/Image.Height);
            fflush(stdout);
        }
    }

    const char *filename = "test.bmp";
    WriteImage(Image, filename);
    
    printf("Raycasting Done....... \n");

    return 0;
}
