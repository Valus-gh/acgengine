/**
 * @file		dds.h
 * @brief	DirectX .dds basic support (to avoid additional dependencies)
 *
 * @author	Achille Peternier (C) 2013-2015 achille.peternier@gmail.com
 */
#pragma once



//////////////
// #INCLUDE //
//////////////

   // Windows:
   #include <iostream>



/////////////
// #DEFINE //
/////////////

   // DDS header:
   #define DDS_MAGICNUMBER    0x20534444

   // DDS header flags:
   #define DDSD_CAPS		      0x1
   #define DDSD_HEIGHT	      0x2
   #define DDSD_WIDTH	      0x4
   #define DDSD_PITCH	      0x8
   #define DDSD_PIXELFORMAT	0x1000
   #define DDSD_MIPMAPCOUNT	0x20000
   #define DDSD_LINEARSIZE		0x80000
   #define DDSD_DEPTH	      0x800000
   #define DDSCAPS_TEXTURE	   0x1000
   #define DDSCAPS_COMPLEX	   0x8
   #define DDSCAPS_MIPMAP	   0x400000

   // DDS pixel formats:
   #define DDPF_ALPHAPIXELS	0x1
   #define DDPF_ALPHA	      0x2
   #define DDPF_FOURCC	      0x4
   #define DDPF_RGB	         0x40
   #define DDPF_YUV	         0x200
   #define DDPF_LUMINANCE	   0x2

   // DDS caps2 flags:
   #define DDSCAPS2_CUBEMAP	         0x200
   #define DDSCAPS2_CUBEMAP_POSITIVEX	0x400
   #define DDSCAPS2_CUBEMAP_NEGATIVEX	0x800
   #define DDSCAPS2_CUBEMAP_POSITIVEY	0x1000
   #define DDSCAPS2_CUBEMAP_NEGATIVEY	0x2000
   #define DDSCAPS2_CUBEMAP_POSITIVEZ	0x4000
   #define DDSCAPS2_CUBEMAP_NEGATIVEZ	0x8000
   #define DDSCAPS2_CUBEMAP_ALL_FACES  0xFC00
   #define DDSCAPS2_VOLUME	         	0x200000



///////////
// ENUMS //
///////////

typedef enum DXGI_FORMAT
{
  DXGI_FORMAT_UNKNOWN                     = 0,
  DXGI_FORMAT_R32G32B32A32_TYPELESS       = 1,
  DXGI_FORMAT_R32G32B32A32_FLOAT          = 2,
  DXGI_FORMAT_R32G32B32A32_UINT           = 3,
  DXGI_FORMAT_R32G32B32A32_SINT           = 4,
  DXGI_FORMAT_R32G32B32_TYPELESS          = 5,
  DXGI_FORMAT_R32G32B32_FLOAT             = 6,
  DXGI_FORMAT_R32G32B32_UINT              = 7,
  DXGI_FORMAT_R32G32B32_SINT              = 8,
  DXGI_FORMAT_R16G16B16A16_TYPELESS       = 9,
  DXGI_FORMAT_R16G16B16A16_FLOAT          = 10,
  DXGI_FORMAT_R16G16B16A16_UNORM          = 11,
  DXGI_FORMAT_R16G16B16A16_UINT           = 12,
  DXGI_FORMAT_R16G16B16A16_SNORM          = 13,
  DXGI_FORMAT_R16G16B16A16_SINT           = 14,
  DXGI_FORMAT_R32G32_TYPELESS             = 15,
  DXGI_FORMAT_R32G32_FLOAT                = 16,
  DXGI_FORMAT_R32G32_UINT                 = 17,
  DXGI_FORMAT_R32G32_SINT                 = 18,
  DXGI_FORMAT_R32G8X24_TYPELESS           = 19,
  DXGI_FORMAT_D32_FLOAT_S8X24_UINT        = 20,
  DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    = 21,
  DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     = 22,
  DXGI_FORMAT_R10G10B10A2_TYPELESS        = 23,
  DXGI_FORMAT_R10G10B10A2_UNORM           = 24,
  DXGI_FORMAT_R10G10B10A2_UINT            = 25,
  DXGI_FORMAT_R11G11B10_FLOAT             = 26,
  DXGI_FORMAT_R8G8B8A8_TYPELESS           = 27,
  DXGI_FORMAT_R8G8B8A8_UNORM              = 28,
  DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
  DXGI_FORMAT_R8G8B8A8_UINT               = 30,
  DXGI_FORMAT_R8G8B8A8_SNORM              = 31,
  DXGI_FORMAT_R8G8B8A8_SINT               = 32,
  DXGI_FORMAT_R16G16_TYPELESS             = 33,
  DXGI_FORMAT_R16G16_FLOAT                = 34,
  DXGI_FORMAT_R16G16_UNORM                = 35,
  DXGI_FORMAT_R16G16_UINT                 = 36,
  DXGI_FORMAT_R16G16_SNORM                = 37,
  DXGI_FORMAT_R16G16_SINT                 = 38,
  DXGI_FORMAT_R32_TYPELESS                = 39,
  DXGI_FORMAT_D32_FLOAT                   = 40,
  DXGI_FORMAT_R32_FLOAT                   = 41,
  DXGI_FORMAT_R32_UINT                    = 42,
  DXGI_FORMAT_R32_SINT                    = 43,
  DXGI_FORMAT_R24G8_TYPELESS              = 44,
  DXGI_FORMAT_D24_UNORM_S8_UINT           = 45,
  DXGI_FORMAT_R24_UNORM_X8_TYPELESS       = 46,
  DXGI_FORMAT_X24_TYPELESS_G8_UINT        = 47,
  DXGI_FORMAT_R8G8_TYPELESS               = 48,
  DXGI_FORMAT_R8G8_UNORM                  = 49,
  DXGI_FORMAT_R8G8_UINT                   = 50,
  DXGI_FORMAT_R8G8_SNORM                  = 51,
  DXGI_FORMAT_R8G8_SINT                   = 52,
  DXGI_FORMAT_R16_TYPELESS                = 53,
  DXGI_FORMAT_R16_FLOAT                   = 54,
  DXGI_FORMAT_D16_UNORM                   = 55,
  DXGI_FORMAT_R16_UNORM                   = 56,
  DXGI_FORMAT_R16_UINT                    = 57,
  DXGI_FORMAT_R16_SNORM                   = 58,
  DXGI_FORMAT_R16_SINT                    = 59,
  DXGI_FORMAT_R8_TYPELESS                 = 60,
  DXGI_FORMAT_R8_UNORM                    = 61,
  DXGI_FORMAT_R8_UINT                     = 62,
  DXGI_FORMAT_R8_SNORM                    = 63,
  DXGI_FORMAT_R8_SINT                     = 64,
  DXGI_FORMAT_A8_UNORM                    = 65,
  DXGI_FORMAT_R1_UNORM                    = 66,
  DXGI_FORMAT_R9G9B9E5_SHAREDEXP          = 67,
  DXGI_FORMAT_R8G8_B8G8_UNORM             = 68,
  DXGI_FORMAT_G8R8_G8B8_UNORM             = 69,
  DXGI_FORMAT_BC1_TYPELESS                = 70,
  DXGI_FORMAT_BC1_UNORM                   = 71,
  DXGI_FORMAT_BC1_UNORM_SRGB              = 72,
  DXGI_FORMAT_BC2_TYPELESS                = 73,
  DXGI_FORMAT_BC2_UNORM                   = 74,
  DXGI_FORMAT_BC2_UNORM_SRGB              = 75,
  DXGI_FORMAT_BC3_TYPELESS                = 76,
  DXGI_FORMAT_BC3_UNORM                   = 77,
  DXGI_FORMAT_BC3_UNORM_SRGB              = 78,
  DXGI_FORMAT_BC4_TYPELESS                = 79,
  DXGI_FORMAT_BC4_UNORM                   = 80,
  DXGI_FORMAT_BC4_SNORM                   = 81,
  DXGI_FORMAT_BC5_TYPELESS                = 82,
  DXGI_FORMAT_BC5_UNORM                   = 83,
  DXGI_FORMAT_BC5_SNORM                   = 84,
  DXGI_FORMAT_B5G6R5_UNORM                = 85,
  DXGI_FORMAT_B5G5R5A1_UNORM              = 86,
  DXGI_FORMAT_B8G8R8A8_UNORM              = 87,
  DXGI_FORMAT_B8G8R8X8_UNORM              = 88,
  DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  = 89,
  DXGI_FORMAT_B8G8R8A8_TYPELESS           = 90,
  DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         = 91,
  DXGI_FORMAT_B8G8R8X8_TYPELESS           = 92,
  DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         = 93,
  DXGI_FORMAT_BC6H_TYPELESS               = 94,
  DXGI_FORMAT_BC6H_UF16                   = 95,
  DXGI_FORMAT_BC6H_SF16                   = 96,
  DXGI_FORMAT_BC7_TYPELESS                = 97,
  DXGI_FORMAT_BC7_UNORM                   = 98,
  DXGI_FORMAT_BC7_UNORM_SRGB              = 99,
  DXGI_FORMAT_AYUV                        = 100,
  DXGI_FORMAT_Y410                        = 101,
  DXGI_FORMAT_Y416                        = 102,
  DXGI_FORMAT_NV12                        = 103,
  DXGI_FORMAT_P010                        = 104,
  DXGI_FORMAT_P016                        = 105,
  DXGI_FORMAT_420_OPAQUE                  = 106,
  DXGI_FORMAT_YUY2                        = 107,
  DXGI_FORMAT_Y210                        = 108,
  DXGI_FORMAT_Y216                        = 109,
  DXGI_FORMAT_NV11                        = 110,
  DXGI_FORMAT_AI44                        = 111,
  DXGI_FORMAT_IA44                        = 112,
  DXGI_FORMAT_P8                          = 113,
  DXGI_FORMAT_A8P8                        = 114,
  DXGI_FORMAT_B4G4R4A4_UNORM              = 115,
  DXGI_FORMAT_P208                        = 130,
  DXGI_FORMAT_V208                        = 131,
  DXGI_FORMAT_V408                        = 132,
  DXGI_FORMAT_ASTC_4X4_UNORM              = 134,
  DXGI_FORMAT_ASTC_4X4_UNORM_SRGB         = 135,
  DXGI_FORMAT_ASTC_5X4_TYPELESS           = 137,
  DXGI_FORMAT_ASTC_5X4_UNORM              = 138,
  DXGI_FORMAT_ASTC_5X4_UNORM_SRGB         = 139,
  DXGI_FORMAT_ASTC_5X5_TYPELESS           = 141,
  DXGI_FORMAT_ASTC_5X5_UNORM              = 142,
  DXGI_FORMAT_ASTC_5X5_UNORM_SRGB         = 143,
  DXGI_FORMAT_ASTC_6X5_TYPELESS           = 145,
  DXGI_FORMAT_ASTC_6X5_UNORM              = 146,
  DXGI_FORMAT_ASTC_6X5_UNORM_SRGB         = 147,
  DXGI_FORMAT_ASTC_6X6_TYPELESS           = 149,
  DXGI_FORMAT_ASTC_6X6_UNORM              = 150,
  DXGI_FORMAT_ASTC_6X6_UNORM_SRGB         = 151,
  DXGI_FORMAT_ASTC_8X5_TYPELESS           = 153,
  DXGI_FORMAT_ASTC_8X5_UNORM              = 154,
  DXGI_FORMAT_ASTC_8X5_UNORM_SRGB         = 155,
  DXGI_FORMAT_ASTC_8X6_TYPELESS           = 157,
  DXGI_FORMAT_ASTC_8X6_UNORM              = 158,
  DXGI_FORMAT_ASTC_8X6_UNORM_SRGB         = 159,
  DXGI_FORMAT_ASTC_8X8_TYPELESS           = 161,
  DXGI_FORMAT_ASTC_8X8_UNORM              = 162,
  DXGI_FORMAT_ASTC_8X8_UNORM_SRGB         = 163,
  DXGI_FORMAT_ASTC_10X5_TYPELESS          = 165,
  DXGI_FORMAT_ASTC_10X5_UNORM             = 166,
  DXGI_FORMAT_ASTC_10X5_UNORM_SRGB        = 167,
  DXGI_FORMAT_ASTC_10X6_TYPELESS          = 169,
  DXGI_FORMAT_ASTC_10X6_UNORM             = 170,
  DXGI_FORMAT_ASTC_10X6_UNORM_SRGB        = 171,
  DXGI_FORMAT_ASTC_10X8_TYPELESS          = 173,
  DXGI_FORMAT_ASTC_10X8_UNORM             = 174,
  DXGI_FORMAT_ASTC_10X8_UNORM_SRGB        = 175,
  DXGI_FORMAT_ASTC_10X10_TYPELESS         = 177,
  DXGI_FORMAT_ASTC_10X10_UNORM            = 178,
  DXGI_FORMAT_ASTC_10X10_UNORM_SRGB       = 179,
  DXGI_FORMAT_ASTC_12X10_TYPELESS         = 181,
  DXGI_FORMAT_ASTC_12X10_UNORM            = 182,
  DXGI_FORMAT_ASTC_12X10_UNORM_SRGB       = 183,
  DXGI_FORMAT_ASTC_12X12_TYPELESS         = 185,
  DXGI_FORMAT_ASTC_12X12_UNORM            = 186,
  DXGI_FORMAT_ASTC_12X12_UNORM_SRGB       = 187,
  DXGI_FORMAT_FORCE_UINT                  = 0xffffffff
} DXGI_FORMAT;

typedef enum D3D10_RESOURCE_DIMENSION
{
  D3D10_RESOURCE_DIMENSION_UNKNOWN    = 0,
  D3D10_RESOURCE_DIMENSION_BUFFER     = 1,
  D3D10_RESOURCE_DIMENSION_TEXTURE1D  = 2,
  D3D10_RESOURCE_DIMENSION_TEXTURE2D  = 3,
  D3D10_RESOURCE_DIMENSION_TEXTURE3D  = 4
} D3D10_RESOURCE_DIMENSION;



/////////////
// STRUCTS //
/////////////

typedef struct
{
   unsigned int dwSize;
   unsigned int dwFlags;
   unsigned int dwFourCC;
   unsigned int dwRGBBitCount;
   unsigned int dwRBitMask;
   unsigned int dwGBitMask;
   unsigned int dwBBitMask;
   unsigned int dwABitMask;
} DDS_PIXELFORMAT;

typedef struct
{
   unsigned int dwSize;
   unsigned int dwFlags;
   unsigned int dwHeight;
   unsigned int dwWidth;
   unsigned int dwPitchOrLinearSize;
   unsigned int dwDepth;
   unsigned int dwMipMapCount;
   unsigned int dwReserved1[11];
   DDS_PIXELFORMAT ddspf;
   unsigned int dwCaps;
   unsigned int dwCaps2;
   unsigned int dwCaps3;
   unsigned int dwCaps4;
   unsigned int dwReserved2;
} DDS_HEADER;

typedef struct
{
  DXGI_FORMAT              dxgiFormat;
  D3D10_RESOURCE_DIMENSION resourceDimension;
  unsigned int             miscFlag;
  unsigned int             arraySize;
  unsigned int             miscFlags2;
} DDS_HEADER10;
