#pragma once
#include <d2d1.h>

#ifdef GIFRESIZER_EXPORTS
#define RESIZER_API _declspec(dllexport)
#else
#define RESIZER_API _declspec(dllimport)
#endif

extern RESIZER_API D2D1_RECT_F Resizer(ID2D1Bitmap* InputBitmap, float first_x, float first_y);
