#pragma once
//Author: yinkaisheng@foxmail.com

#ifdef __EXPORT_DLL__
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    DLL_EXPORT void Initialize();

    DLL_EXPORT void Uninitialize();

    DLL_EXPORT UINT GetMonitorsRect(int* pInt, int size, int dpiAwareness);

    DLL_EXPORT size_t BitmapCreate(int width, int height);

    DLL_EXPORT size_t BitmapFromWindow(size_t wnd, int left, int top, int right, int bottom);

    DLL_EXPORT size_t BitmapFromFile(LPCTSTR path);

    DLL_EXPORT BOOL BitmapToFile(size_t bitmap, LPCTSTR path, LPCTSTR gdiplusFormat);

    DLL_EXPORT void BitmapRelease(size_t bitmap);

    DLL_EXPORT UINT BitmapGetWidthAndHeight(size_t bitmap);

    DLL_EXPORT UINT BitmapGetPixel(size_t bitmap, int x, int y);

    DLL_EXPORT BOOL BitmapSetPixel(size_t bitmap, int x, int y, UINT argb);

    DLL_EXPORT BOOL BitmapGetPixelsHorizontally(size_t bitmap, int x, int y, UINT* array, int size);

    DLL_EXPORT BOOL BitmapGetPixelsVertically(size_t bitmap, int x, int y, UINT* array, int size);

    DLL_EXPORT BOOL BitmapSetPixelsHorizontally(size_t bitmap, int x, int y, UINT* array, int size);

    DLL_EXPORT BOOL BitmapSetPixelsVertically(size_t bitmap, int x, int y, UINT* array, int size);

	DLL_EXPORT BOOL BitmapGetPixelsOfRect(size_t bitmap, int x, int y, int width, int height, UINT* array);

	DLL_EXPORT BOOL BitmapSetPixelsOfRect(size_t bitmap, int x, int y, int width, int height, UINT* array);

#ifdef __cplusplus
}
#endif

