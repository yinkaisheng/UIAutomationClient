//Author: yinkaisheng@live.com
#include "stdafx.h"
#include <memory>
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

struct LibraryDeleter
{
    typedef HMODULE pointer;
    void operator()(HMODULE h) { ::FreeLibrary(h); }
};

struct MonitorsInfo
{
    int* pRect;
    int size;
    int index;
};

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

    BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT pRect, LPARAM lParam)
    {
        MonitorsInfo &monitorsInfo = *((MonitorsInfo*)lParam);
        if (monitorsInfo.index > monitorsInfo.size - 4)
        {
            return FALSE;
        }

        monitorsInfo.pRect[monitorsInfo.index++] = pRect->left;
        monitorsInfo.pRect[monitorsInfo.index++] = pRect->top;
        monitorsInfo.pRect[monitorsInfo.index++] = pRect->right;
        monitorsInfo.pRect[monitorsInfo.index++] = pRect->bottom;

        return TRUE;
    }

    DLL_EXPORT UINT GetMonitorsRect(int* pInt, int size, int dpiAwareness)
    {
        if (pInt == nullptr || size <= 0)
        {
            return 0;
        }

        std::unique_ptr<HMODULE, LibraryDeleter> dll(::LoadLibraryW(L"Shcore.dll"));

        if (dll)
        {
            typedef HRESULT(WINAPI *PFN_SetProcessDpiAwareness)(int value);
            PFN_SetProcessDpiAwareness pSetProcessDpiAwareness = (PFN_SetProcessDpiAwareness)GetProcAddress(dll.get(), "SetProcessDpiAwareness");

            if (pSetProcessDpiAwareness)
            {
                pSetProcessDpiAwareness(dpiAwareness);
            }
        }

        MonitorsInfo monitorsInfo{ pInt, size, 0 };
        EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)&monitorsInfo);

        return monitorsInfo.index / 4;
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

    DLL_EXPORT size_t BitmapResizedFrom(size_t bitmap, int newWidth, int newHeight)
    {
        if (bitmap && newWidth > 0 && newHeight > 0)
        {
            Bitmap* pSrcBitmap = reinterpret_cast<Bitmap*>(bitmap);
            Bitmap* pBitmap = new Bitmap(newWidth, newHeight, PixelFormat32bppARGB);
            Graphics graphics{ pBitmap };
            if (Status::Ok == graphics.DrawImage(pSrcBitmap, 0, 0, newWidth, newHeight))
            {
                return reinterpret_cast<size_t>(pBitmap);
            }
            else
            {
                delete pBitmap;
            }
        }

        return 0;
    }

    DLL_EXPORT size_t BitmapRotatedFrom(size_t bitmap, int angle, UINT backgroundArgb)
    {
        if (bitmap == 0)
        {
            return 0;
        }

        Bitmap* pSrcBitmap = reinterpret_cast<Bitmap*>(bitmap);
        int srcWidth = pSrcBitmap->GetWidth();
        int srcHeight = pSrcBitmap->GetHeight();
        double radian = 0.0174532925 * angle;
        double dcos = cos(radian);
        double dsin = sin(radian);
        int newWidth = static_cast<int>(max(abs(srcWidth * dcos - srcHeight * dsin), abs(srcWidth * dcos + srcHeight * dsin)));
        int newHeight = static_cast<int>(max(abs(srcWidth * dsin - srcHeight * dcos), abs(srcWidth * dsin + srcHeight * dcos)));
        Bitmap *pBitmap = new Bitmap(newWidth, newHeight);
        Graphics graphics{ pBitmap };
        graphics.Clear(Color{ backgroundArgb });
        Point centerPoint{ newWidth / 2, newHeight / 2 };
        graphics.TranslateTransform(centerPoint.X, centerPoint.Y);
        graphics.RotateTransform(angle);
        graphics.TranslateTransform(-centerPoint.X, -centerPoint.Y);
        graphics.DrawImage(pSrcBitmap, (newWidth - srcWidth) / 2, (newHeight - srcHeight) / 2, srcWidth, srcHeight);
        if (Status::Ok == graphics.DrawImage(pSrcBitmap, (newWidth - srcWidth) / 2, (newHeight - srcHeight) / 2, srcWidth, srcHeight))
        {
            return reinterpret_cast<size_t>(pBitmap);
        }
        else
        {
            delete pBitmap;
        }

        return 0;
    }

    DLL_EXPORT BOOL BitmapRotateFlip(size_t bitmap, int rotateFlip)
    {
        if (bitmap == 0)
        {
            return 0;
        }

        Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
        return pBitmap->RotateFlip(static_cast<RotateFlipType>(rotateFlip)) == Status::Ok;
    }

    DLL_EXPORT size_t BitmapFromWindow(size_t wnd, int left, int top, int right, int bottom)
    {
        HWND hWnd = (HWND)wnd;
        RECT rcWnd;
        GetWindowRect(hWnd, &rcWnd);

        HDC hdc = GetWindowDC(hWnd);
        HBITMAP hBitmap = CopyDC2Bitmap(hdc, left, top, right, bottom);

        Bitmap* pBitmap = Bitmap::FromHBITMAP(hBitmap, nullptr);

        DeleteObject(hBitmap);
        ReleaseDC(hWnd, hdc);

        return reinterpret_cast<size_t>(pBitmap);
    }

    DLL_EXPORT size_t BitmapFromHBITMAP(size_t hBitmap)
    {
        if (hBitmap == 0)
        {
            return 0;
        }

        HBITMAP hBmp = reinterpret_cast<HBITMAP>(hBitmap);
        Bitmap *pBitmap = Bitmap::FromHBITMAP(hBmp, nullptr);
        return reinterpret_cast<size_t>(pBitmap);
    }

    DLL_EXPORT size_t BitmapToHBITMAP(size_t bitmap, UINT backArgb)
    {
        if (bitmap == 0)
        {
            return 0;
        }

        Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
        HBITMAP hBitmap{};
        Gdiplus::Color back{backArgb>>24, (backArgb>>16)&&0xFF, (backArgb>>8)&&0xFF, backArgb&&0xFF};
        pBitmap->GetHBITMAP(back, &hBitmap);
        return reinterpret_cast<size_t>(hBitmap);
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
                if (Status::Ok == pBitmap->Save(path, &imgClsId, nullptr))
                {
                    return TRUE;
                }
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
            Gdiplus::Rect rect{x, y, size, 1};
            if (x + size > width)
            {
                rect.X = 0;
                rect.Width = width;
                rect.Height += (size + x - 1) / width;
            }
            Gdiplus::BitmapData bmpData{};
            pBitmap->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeRead, PixelFormat32bppARGB, &bmpData);
            memcpy(array, x + size > width ? (char*)bmpData.Scan0 + x * 4 : (char*)bmpData.Scan0, size * 4);
            pBitmap->UnlockBits(&bmpData);
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
            Gdiplus::Rect rect(x, y, size, 1);
            if (x + size > width)
            {
                rect.X = 0;
                rect.Width = width;
                rect.Height += (size + x - 1) / width;
            }

            Gdiplus::BitmapData bmpData{};
            pBitmap->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeRead, PixelFormat32bppARGB, &bmpData);
            memcpy(x + size > width ? (char*)bmpData.Scan0 + x * 4 : (char*)bmpData.Scan0, array, size * 4);
            pBitmap->UnlockBits(&bmpData);
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
            Gdiplus::Rect rect(x,y,width,height);
            Gdiplus::BitmapData bmpData{};
            bmpData.Width = rect.Width;
            bmpData.Height = rect.Height;
            bmpData.PixelFormat = PixelFormat32bppARGB;
            bmpData.Stride = rect.Width * 4;
            bmpData.Scan0 = array;
            pBitmap->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeUserInputBuf | Gdiplus::ImageLockMode::ImageLockModeRead,
                PixelFormat32bppARGB, &bmpData);
            pBitmap->UnlockBits(&bmpData);
            return TRUE;
        }

        return FALSE;
    }

    DLL_EXPORT BOOL BitmapSetPixelsOfRect(size_t bitmap, int x, int y, int width, int height, UINT* array)
    {
        if (bitmap && array)
        {
            Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
            Gdiplus::Rect rect(x, y, width, height);
            Gdiplus::BitmapData bmpData{};
            bmpData.Width = rect.Width;
            bmpData.Height = rect.Height;
            bmpData.PixelFormat = PixelFormat32bppARGB;
            bmpData.Stride = rect.Width * 4;
            bmpData.Scan0 = array;
            pBitmap->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeUserInputBuf | Gdiplus::ImageLockMode::ImageLockModeWrite,
                PixelFormat32bppARGB, &bmpData);
            pBitmap->UnlockBits(&bmpData);
            return TRUE;
        }

        return FALSE;
    }


#ifdef __cplusplus
}
#endif
