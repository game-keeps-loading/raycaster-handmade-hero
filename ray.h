#ifndef RAY_H
#define RAY_H

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<float.h>
#include<math.h>
#include<time.h>
#include<assert.h>
#include<windows.h>

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

#define F32Max FLT_MAX
#define F32Min FLT_MIN
#define internal static
#define global static

#define U32Max ((u32)-1)
#define MAX_WIDTH 3840
#define MAX_HEIGHT 2160
#define Pi32 3.14159265359f
#define Tau32 6.2831853071f

#define ArrayCount(Array) (sizeof(Array)/sizeof(Array[0]))

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

struct random_series {
    u32 State;
};

struct work_order {
    world *World;
    image_u32 Image;
    u32 XMin;
    u32 YMin;
    u32 OnePastXMax;
    u32 OnePastYMax;

    random_series Entropy;
};

struct work_queue {
    u32 WorkOrderCount;
    work_order *WorkOrders;
    u32 RaysPerPixel;
    u32 MaxBounceCount;

    volatile u64 NextWorkOrderIndex;
    volatile u64 BonucesComputed;
    volatile u64 TileRetiredCount;
};


struct cast_state {
    world *World;
    u32 RaysPerPixel;
    u32 MaxBounceCount;
        
    f32 FilmX;
    f32 FilmY;
    f32 HalfPixW;
    f32 HalfPixH;
        
    v3  FilmCenter;
    f32 HalfFilmW;
    f32 HalfFilmH;
    
    v3  CameraX;
    v3  CameraY;
    v3  CameraZ;
    v3  CameraPosiition;

    random_series Series;
    v3  FinalColor;
    u64 BouncesComputed;
};

u32 xorshift32(random_series *Series) {
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	u32 x = Series->State;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
    return Series->State=x;
}

#if defined(_WIN32)
u32 GetCPUCount() {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}
#elif defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
u32 GetCPUCount() {
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    return (n > 0) ? (unsigned int)n : 1;
}
#else
#include <thread>
u32 GetCPUCount() {
    unsigned int n = std::thread::hardware_concurrency();
    return (n > 0) ? n : 1;
}
#endif

internal u64 LockedADDAndReturnPreviousValue(u64 volatile *Value, u64 Addend);
internal DWORD WINAPI WorkerThread(void *IpParameter) ;
internal void CreateWorkThread(void *Parameter);

#endif