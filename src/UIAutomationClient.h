#pragma once
#include <windows.h>
#include <stdint.h>

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

#ifdef _DEBUG
	DLL_EXPORT int GetParentProcessId(int nProcessId);
#endif

	DLL_EXPORT UINT GetMonitorsRect(int* pInt, int size, int dpiAwareness);

	DLL_EXPORT int GetImageEncoderClsid(const wchar_t* format, CLSID* pClsid);

	DLL_EXPORT size_t BitmapCreate(int width, int height);

	DLL_EXPORT size_t BitmapClone(size_t bitmap, int x, int y, int width, int height);

	DLL_EXPORT size_t BitmapResize(size_t bitmap, int newWidth, int newHeight);

	DLL_EXPORT size_t BitmapRotate(size_t bitmap, float angle, UINT backgroundArgb = 0xFFFFFFFF);

	DLL_EXPORT size_t BitmapRotateWithSameSize(size_t bitmap, float dx, float dy, float angle, UINT backgroundArgb = 0xFFFFFFFF);

	DLL_EXPORT UINT BitmapRotateFlip(size_t bitmap, int rotateFlip);

	DLL_EXPORT size_t BitmapFromWindow(size_t wnd, int left, int top, int right, int bottom);

	DLL_EXPORT size_t BitmapFromHBITMAP(size_t hBitmap);

	DLL_EXPORT size_t BitmapToHBITMAP(size_t bitmap, UINT backgroundArgb = 0xFFFFFFFF);

	DLL_EXPORT size_t BitmapFromFile(LPCTSTR path);

	DLL_EXPORT size_t BitmapFromBytes(const BYTE* data, UINT len, BOOL useEmbeddedColorManagement = 0);

	DLL_EXPORT UINT BitmapGetRawFormat(size_t bitmap, UINT* format);

	DLL_EXPORT UINT BitmapToFile(size_t bitmap, LPCTSTR path, LPCTSTR gdiplusFormat, UINT quality);

	DLL_EXPORT UINT BitmapToStream(size_t bitmap, size_t* outStream, LPCTSTR gdiplusFormat, UINT quality);

	DLL_EXPORT void BitmapRelease(size_t bitmap);

	DLL_EXPORT uint64_t BitmapGetWidthAndHeight(size_t bitmap);

	DLL_EXPORT UINT BitmapGetPixel(size_t bitmap, int x, int y);

	DLL_EXPORT UINT BitmapSetPixel(size_t bitmap, int x, int y, UINT argb);

	DLL_EXPORT UINT BitmapGetPixelsHorizontally(size_t bitmap, int x, int y, UINT* array, int size);

	DLL_EXPORT UINT BitmapSetPixelsHorizontally(size_t bitmap, int x, int y, UINT* array, int size);

	DLL_EXPORT UINT BitmapGetPixelsVertically(size_t bitmap, int x, int y, UINT* array, int size);

	DLL_EXPORT UINT BitmapSetPixelsVertically(size_t bitmap, int x, int y, UINT* array, int size);

	DLL_EXPORT UINT BitmapGetPixelsOfRect(size_t bitmap, int x, int y, int width, int height, UINT* array);

	DLL_EXPORT UINT BitmapSetPixelsOfRect(size_t bitmap, int x, int y, int width, int height, UINT* array);

	DLL_EXPORT UINT MultiBitmapGetFrameCount(size_t bitmap);

	DLL_EXPORT UINT MultiBitmapGetFrameDelaySize(size_t bitmap);

	DLL_EXPORT UINT MultiBitmapGetFrameDelay(size_t bitmap, char* delay, UINT size, int* valueOffset);

	DLL_EXPORT UINT MultiBitmapSelectActiveFrame(size_t bitmap, UINT frameIndex);

	DLL_EXPORT UINT MultiBitmapToFile(size_t* bitmaps, ULONG* delay, UINT size, LPCTSTR path, LPCTSTR format);

	DLL_EXPORT UINT GetStreamSize(size_t stream);

	DLL_EXPORT UINT CopyStreamData(size_t stream, char* outData, UINT outSize);

	DLL_EXPORT void ReleaseStream(size_t stream);
#ifdef __cplusplus
}
#endif

