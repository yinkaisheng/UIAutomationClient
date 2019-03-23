//Author: yinkaisheng@live.com
#include "stdafx.h"
#include <tchar.h>
#include <stdio.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

#define __EXPORT_DLL__
#include "UIAutomationClient.h"

#if (_MSC_VER <= 1500)
#define nullptr 0
#endif

ULONG_PTR g_nGdiPlusToken = 0;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        {
            break;
        }
    case DLL_THREAD_ATTACH:
        {
            break;
        }
    case DLL_THREAD_DETACH:
        {
            break;
        }
    case DLL_PROCESS_DETACH:
        {
            break;
        }
    }
    return TRUE;
}

#ifdef __cplusplus
extern "C"
{
#endif

    DLL_EXPORT void Initialize()
    {
        if (!g_nGdiPlusToken)
        {
            GdiplusStartupInput gdiplusStartupInput;
            GdiplusStartup(&g_nGdiPlusToken, &gdiplusStartupInput, nullptr);
        }
    }

    DLL_EXPORT void Uninitialize()
    {
        if (g_nGdiPlusToken)
        {
            GdiplusShutdown(g_nGdiPlusToken);
            g_nGdiPlusToken = 0;
        }
    }
	
    int GetImageEncoderClsid(const wchar_t* format, CLSID* pClsid)
    {
        UINT num = 0;          // number of image encoders
        UINT size = 0;         // size of the image encoder array in bytes

        ImageCodecInfo* pImageCodecInfo = nullptr;

        GetImageEncodersSize(&num, &size);
        if(size == 0)
            return -1;  // Failure

        pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
        if(pImageCodecInfo == nullptr)
            return -1;  // Failure

        GetImageEncoders(num, size, pImageCodecInfo);

        for(UINT j = 0; j < num; ++j)
        {
            //wprintf(L"%s\n", pImageCodecInfo[j].MimeType);
            if(wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
            {
                *pClsid = pImageCodecInfo[j].Clsid;
                free(pImageCodecInfo);
                return j;  // Success
            }
        }

        free(pImageCodecInfo);
        return -1;  // Failure
    }

    HBITMAP CopyDC2Bitmap(HDC hdc, int nLeft, int nTop, int nRight, int nBottom)
    {
        int nWidth = nRight - nLeft;
        int nHeight = nBottom - nTop;
        HDC hMemDC = CreateCompatibleDC(hdc);
        HBITMAP hBitmap = CreateCompatibleBitmap(hdc, nWidth, nHeight);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
        BitBlt(hMemDC, 0, 0, nWidth, nHeight, hdc, nLeft, nTop, SRCCOPY);
        hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);
        DeleteDC(hMemDC);
        return hBitmap;
    }

    DLL_EXPORT size_t BitmapCreate(int width, int height)
    {
        Bitmap* pBitmap = new Bitmap(width, height, PixelFormat32bppARGB);
        return reinterpret_cast<size_t>(pBitmap);
    }

    DLL_EXPORT size_t BitmapFromWindow(size_t wnd, int left, int top, int right, int bottom)
    {
        HWND hWnd = (HWND)wnd;
        RECT rcWnd;
        GetWindowRect(hWnd, &rcWnd);

        if (right <= 0)
        {
            right = rcWnd.right - rcWnd.left;
        }

        if (bottom <= 0)
        {
            bottom = rcWnd.bottom - rcWnd.top;
        }

        HDC hdc = GetWindowDC(hWnd);
        HBITMAP hBitmap = CopyDC2Bitmap(hdc, left, top, right, bottom);

        Bitmap* pBitmap = Bitmap::FromHBITMAP(hBitmap, nullptr);

        DeleteObject(hBitmap);
        ReleaseDC(hWnd, hdc);

        return reinterpret_cast<size_t>(pBitmap);
    }

    DLL_EXPORT size_t BitmapFromFile(LPCTSTR path)
    {
        Bitmap* pBitmap = Bitmap::FromFile(path);
        return reinterpret_cast<size_t>(pBitmap);
    }

    DLL_EXPORT BOOL BitmapToFile(size_t bitmap, LPCTSTR path, LPCTSTR gdiplusFormat)
    {
        if (bitmap)
        {
            Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
            CLSID imgClsId;
            int retVal = GetImageEncoderClsid(gdiplusFormat, &imgClsId);
            if (retVal >= 0)
            {
                pBitmap->Save(path, &imgClsId, nullptr);
                return TRUE;
            }
        }

        return FALSE;
    }

    DLL_EXPORT void BitmapRelease(size_t bitmap)
    {
        if (bitmap)
        {
            Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
            delete pBitmap;
        }
    }

    DLL_EXPORT UINT BitmapGetWidthAndHeight(size_t bitmap)
    {
        if (bitmap)
        {
            Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
            return pBitmap->GetWidth() | (pBitmap->GetHeight() << 16);
        }

        return 0;
    }

    DLL_EXPORT UINT BitmapGetPixel(size_t bitmap, int x, int y)
    {
        if (bitmap)
        {
            Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
            Color color;
            pBitmap->GetPixel(x, y, &color);
            return color.GetValue();
        }

        return 0;
    }

    DLL_EXPORT BOOL BitmapSetPixel(size_t bitmap, int x, int y, UINT argb)
    {
        if (bitmap)
        {
            Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
            Color color(argb);
            pBitmap->SetPixel(x, y, color);
            return TRUE;
        }

        return FALSE;
    }

    DLL_EXPORT BOOL BitmapGetPixelsHorizontally(size_t bitmap, int x, int y, UINT* array, int size)
    {
        if (bitmap && array)
        {
            Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
            int width = static_cast<int>(pBitmap->GetWidth());
			Color color;
			for (int n = 0; n < size; ++n)
            {
                pBitmap->GetPixel(x, y, &color);
                array[n] = color.GetValue();
                if (++x >= width)
                {
                    x = 0;
                    ++y;
                }
            }
            return TRUE;
        }

        return FALSE;
    }

    DLL_EXPORT BOOL BitmapGetPixelsVertically(size_t bitmap, int x, int y, UINT* array, int size)
    {
        if (bitmap && array)
        {
            Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
            int height = static_cast<int>(pBitmap->GetHeight());
			Color color;
			for (int n = 0; n < size; ++n)
            {
                pBitmap->GetPixel(x, y, &color);
                array[n] = color.GetValue();
                if (++y >= height)
                {
                    y = 0;
                    ++x;
                }
            }
            return TRUE;
        }

        return FALSE;
    }

    DLL_EXPORT BOOL BitmapSetPixelsHorizontally(size_t bitmap, int x, int y, UINT* array, int size)
    {
        if (bitmap && array)
        {
            Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
            int width = static_cast<int>(pBitmap->GetWidth());
			Color color;
			for (int n = 0; n < size; ++n)
            {
                color.SetValue(array[n]);
                pBitmap->SetPixel(x, y, color);
                if (++x >= width)
                {
                    x = 0;
                    ++y;
                }
            }
            return TRUE;
        }

        return FALSE;
    }

    DLL_EXPORT BOOL BitmapSetPixelsVertically(size_t bitmap, int x, int y, UINT* array, int size)
    {
        if (bitmap && array)
        {
            Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
            int height = static_cast<int>(pBitmap->GetHeight());
			Color color;
			for (int n = 0; n < size; ++n)
            {
				color.SetValue(array[n]);
                pBitmap->SetPixel(x, y, color);
                if (++y >= height)
                {
                    y = 0;
                    ++x;
                }
            }
            return TRUE;
        }

        return FALSE;
    }

	DLL_EXPORT BOOL BitmapGetPixelsOfRect(size_t bitmap, int x, int y, int width, int height, UINT* array)
	{
		if (bitmap && array)
		{
			Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
			Color color;
			int index = 0;
			for (int col = y; col < y + height; ++col)
			{
				for (int row = x; row < x + width; ++row)
				{
					pBitmap->GetPixel(row, col, &color);
					array[index++] = color.GetValue();
				}
			}
			return TRUE;
		}

		return FALSE;
	}

	DLL_EXPORT BOOL BitmapSetPixelsOfRect(size_t bitmap, int x, int y, int width, int height, UINT* array)
	{
		if (bitmap && array)
		{
			Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
			Color color;
			int index = 0;
			for (int col = y; col < y + height; ++col)
			{
				for (int row = x; row < x + width; ++row)
				{
					color.SetValue(array[index++]);
					pBitmap->SetPixel(row, col, color);
				}
			}
			return TRUE;
		}

		return FALSE;
	}

#ifdef __cplusplus
}
#endif
