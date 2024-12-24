//Author: yinkaisheng@live.com
#define __EXPORT_DLL__
#include "UIAutomationClient.h"
#include <memory>
#include <vector>
#include <tchar.h>
#include <stdio.h>
#include <gdiplus.h>
#include <winternl.h>
#include <shlwapi.h>
#include "gif.h"
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "ShLwApi.lib")


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

#ifdef _DEBUG
	DLL_EXPORT int GetParentProcessId(int nProcessId)
	{
		HANDLE hProcess = nullptr;

		if (-1 == nProcessId)
		{
			hProcess = GetCurrentProcess();
		}
		else
		{
			hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, nProcessId);
		}

		if (hProcess)
		{
			typedef NTSTATUS(WINAPI* pfnNtQueryInformationProcess)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
			HMODULE hMod = GetModuleHandle(TEXT("ntdll.dll"));
			pfnNtQueryInformationProcess ntQueryInformationProcess =
				reinterpret_cast<pfnNtQueryInformationProcess>(GetProcAddress(hMod, "NtQueryInformationProcess"));
			PROCESS_BASIC_INFORMATION stInfo = { 0 };
			DWORD dwRetnLen = 0;
			NTSTATUS nRet = ntQueryInformationProcess(hProcess, ProcessBasicInformation, &stInfo, sizeof(stInfo), &dwRetnLen);
			if (nRet == 0 && stInfo.PebBaseAddress)
			{
				PEB peb{}; //size 712, 472
				size_t offset = (size_t)&peb.ProcessParameters - (size_t)&peb; //64bit:32, 32bit: 16
				SIZE_T outSize = 0;
				if (ReadProcessMemory(hProcess, stInfo.PebBaseAddress, &peb, sizeof(peb), &outSize))
				{
					if (peb.ProcessParameters)
					{
						RTL_USER_PROCESS_PARAMETERS upp{ }; //size 128, 72
						outSize = 0;
						if (ReadProcessMemory(hProcess, peb.ProcessParameters, &upp, sizeof(upp), &outSize))
						{
							WCHAR wbuf[MAX_PATH]{};
							outSize = 0;
							ReadProcessMemory(hProcess, upp.ImagePathName.Buffer, wbuf, upp.ImagePathName.Length, &outSize);
							outSize = 0;
							ReadProcessMemory(hProcess, upp.CommandLine.Buffer, wbuf, upp.CommandLine.Length, &outSize);
							//wbuf[outSize/2] = 0;
							outSize = 0;
						}
						
					}
				}
			}
			CloseHandle(hProcess);

			if (0 == nRet)
			{
				return static_cast<int>(reinterpret_cast<size_t>(stInfo.Reserved3));
			}
		}

		return -1;
	}
#endif

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

	DLL_EXPORT int GetImageEncoderClsid(const wchar_t* format, CLSID* pClsid)
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
#ifndef NDEBUG
			wprintf(L"%d, %s\n", j, pImageCodecInfo[j].MimeType);
#endif
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

	DLL_EXPORT size_t BitmapClone(size_t bitmap, int x, int y, int width, int height)
	{
		if (bitmap == 0)
		{
			return 0;
		}
		Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
		Bitmap* pCloned = pBitmap->Clone(x, y, width, height, PixelFormat32bppARGB);
		return reinterpret_cast<size_t>(pCloned);
	}

	DLL_EXPORT size_t BitmapResize(size_t bitmap, int newWidth, int newHeight)
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

	DLL_EXPORT size_t BitmapRotate(size_t bitmap, float angle, UINT backgroundArgb)
	{
		if (bitmap == 0)
		{
			return 0;
		}

		Bitmap* pSrcBitmap = reinterpret_cast<Bitmap*>(bitmap);
		int srcWidth = pSrcBitmap->GetWidth();
		int srcHeight = pSrcBitmap->GetHeight();
		float radian = 0.017453f * angle;
		float fcos = cos(radian);
		float fsin = sin(radian);
		int newWidth = static_cast<int>(round(max(abs(srcWidth * fcos - srcHeight * fsin), abs(srcWidth * fcos + srcHeight * fsin))));
		int newHeight = static_cast<int>(round(max(abs(srcWidth * fsin - srcHeight * fcos), abs(srcWidth * fsin + srcHeight * fcos))));
		Bitmap *pBitmap = new Bitmap(newWidth, newHeight);
		Graphics graphics{ pBitmap };
		graphics.Clear(Color{ backgroundArgb });
		PointF centerPoint{ newWidth / 2.0f, newHeight / 2.0f };
		graphics.TranslateTransform(static_cast<REAL>(centerPoint.X), static_cast<REAL>(centerPoint.Y));
		graphics.RotateTransform(static_cast<REAL>(angle));
		graphics.TranslateTransform(static_cast<REAL>(-centerPoint.X), static_cast<REAL>(-centerPoint.Y));
		if (Status::Ok == graphics.DrawImage(pSrcBitmap, (newWidth - srcWidth) / 2.0f, (newHeight - srcHeight) / 2.0f,
			static_cast<REAL>(srcWidth), static_cast<REAL>(srcHeight)))
		{
			return reinterpret_cast<size_t>(pBitmap);
		}
		else
		{
			delete pBitmap;
		}

		return 0;
	}

	DLL_EXPORT size_t BitmapRotateWithSameSize(size_t bitmap, float dx, float dy, float angle, UINT backgroundArgb)
	{
		// https://learn.microsoft.com/en-us/windows/win32/gdiplus/-gdiplus-rotating-reflecting-and-skewing-images-use
		if (bitmap == 0)
		{
			return 0;
		}

		Bitmap* pSrcBitmap = reinterpret_cast<Bitmap*>(bitmap);
		UINT srcWidth = pSrcBitmap->GetWidth();
		UINT srcHeight = pSrcBitmap->GetHeight();
		Bitmap* pBitmap = new Bitmap(srcWidth, srcHeight);
		Graphics graphics{ pBitmap };
		graphics.Clear(Color{ backgroundArgb });

		float radian = 0.017453f * angle;
		float fcos = cos(radian);
		float fsin = sin(radian);
		PointF leftTop;
		leftTop.X = round(dx + (0 - dx) * fcos - (0 - dy) * fsin);
		leftTop.Y = round(dy + (0 - dy) * fcos + (0 - dx) * fsin);
		PointF rightTop;
		rightTop.X = round(dx + (srcWidth - dx) * fcos - (0 - dy) * fsin);
		rightTop.Y = round(dy + (0 - dy) * fcos + (srcWidth - dx) * fsin);
		PointF leftBottom;
		leftBottom.X = round(dx + (0 - dx) * fcos - (srcHeight - dy) * fsin);
		leftBottom.Y = round(dy + (srcHeight - dy) * fcos + (0 - dx) * fsin);
		std::vector<PointF> points;
		points.push_back(leftTop);
		points.push_back(rightTop);
		points.push_back(leftBottom);

#ifdef _DEBUG
		printf("point1: %.3f, %.3f\n", leftTop.X, leftTop.Y);
		printf("point2: %.3f, %.3f\n", rightTop.X, rightTop.Y);
		printf("point3: %.3f, %.3f\n\n", leftBottom.X, leftBottom.Y);

		//printf("point1: %d, %d\n", leftTop.X, leftTop.Y);
		//printf("point2: %d, %d\n", rightTop.X, rightTop.Y);
		//printf("point3: %d, %d\n\n", leftBottom.X, leftBottom.Y);
#endif

		if (Status::Ok == graphics.DrawImage(pSrcBitmap, points.data(), 3))
		{
			return reinterpret_cast<size_t>(pBitmap);
		}
		else
		{
			delete pBitmap;
		}

		return 0;
	}

	DLL_EXPORT UINT BitmapRotateFlip(size_t bitmap, int rotateFlip)
	{
		if (bitmap == 0)
		{
			return 0;
		}

		Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
		Status status = pBitmap->RotateFlip(static_cast<RotateFlipType>(rotateFlip));
		return status;
	}

	DLL_EXPORT size_t BitmapFromWindow(size_t wnd, int left, int top, int right, int bottom)
	{
		HWND hWnd = (HWND)wnd;
		RECT rcWnd;
		if (!GetWindowRect(hWnd, &rcWnd))
		{
			return 0;
		}

		HDC hdc = GetWindowDC(hWnd);
		HBITMAP hBitmap = CopyDC2Bitmap(hdc, left, top, right, bottom);

		Bitmap* pBitmap = Bitmap::FromHBITMAP(hBitmap, nullptr);

		DeleteObject(hBitmap);
		ReleaseDC(hWnd, hdc);

		if (Status::Ok == pBitmap->GetLastStatus())
		{
			return reinterpret_cast<size_t>(pBitmap);
		}
		else
		{
			delete pBitmap;
		}

		return 0;
	}

	DLL_EXPORT size_t BitmapFromHBITMAP(size_t hBitmap)
	{
		if (hBitmap == 0)
		{
			return 0;
		}

		HBITMAP hBmp = reinterpret_cast<HBITMAP>(hBitmap);
		Bitmap *pBitmap = Bitmap::FromHBITMAP(hBmp, nullptr);

		if (Status::Ok == pBitmap->GetLastStatus())
		{
			return reinterpret_cast<size_t>(pBitmap);
		}
		else
		{
			delete pBitmap;
		}
		
		return 0;
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

		if (Status::Ok == pBitmap->GetLastStatus())
		{
			return reinterpret_cast<size_t>(pBitmap);
		}
		else
		{
			delete pBitmap;
		}

		return 0;
	}

	DLL_EXPORT size_t BitmapFromBytes(const BYTE* data, UINT len, BOOL useEmbeddedColorManagement)
	{
		IStream* pStream = nullptr;
		pStream = SHCreateMemStream(data, len);
		if (pStream == nullptr) {
			return 0;
		}

		Bitmap* pBitmap = new Bitmap(pStream, useEmbeddedColorManagement);
		pStream->Release();

		if (Status::Ok == pBitmap->GetLastStatus())
		{
			return reinterpret_cast<size_t>(pBitmap);
		}
		else
		{
			delete pBitmap;
		}

		return 0;
	}

	enum ImageFormat {
		Undefined,
		MemoryBMP,
		BMP,
		EMF,
		WMF,
		JPEG,
		PNG,
		GIF,
		TIFF,
		EXIF,
		Icon,
	};

	DLL_EXPORT UINT BitmapGetRawFormat(size_t bitmap, UINT *format)
	{
		*format = ImageFormat::Undefined;
		if (bitmap == 0)
		{
			return Status::InvalidParameter;
		}

		Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
		GUID guid{};
		Status status = pBitmap->GetRawFormat(&guid);
		if (status != Status::Ok)
		{
			return status;
		}

		if (memcmp((void*)&ImageFormatMemoryBMP, &guid, sizeof(GUID)) == 0)
		{
			*format = ImageFormat::MemoryBMP;
		}
		else if (memcmp((void*)&ImageFormatBMP, &guid, sizeof(GUID)) == 0)
		{
			*format = ImageFormat::BMP;
		}
		else if (memcmp((void*)&ImageFormatJPEG, &guid, sizeof(GUID)) == 0)
		{
			*format = ImageFormat::JPEG;
		}
		else if (memcmp((void*)&ImageFormatPNG, &guid, sizeof(GUID)) == 0)
		{
			*format = ImageFormat::PNG;
		}
		else if (memcmp((void*)&ImageFormatGIF, &guid, sizeof(GUID)) == 0)
		{
			*format = ImageFormat::GIF;
		}
		else if (memcmp((void*)&ImageFormatTIFF, &guid, sizeof(GUID)) == 0)
		{
			*format = ImageFormat::TIFF;
		}
		else if (memcmp((void*)&ImageFormatIcon, &guid, sizeof(GUID)) == 0)
		{
			*format = ImageFormat::Icon;
		}
		else if (memcmp((void*)&ImageFormatEXIF, &guid, sizeof(GUID)) == 0)
		{
			*format = ImageFormat::EXIF;
		}
		else if (memcmp((void*)&ImageFormatEMF, &guid, sizeof(GUID)) == 0)
		{
			*format = ImageFormat::EMF;
		}
		else if (memcmp((void*)&ImageFormatWMF, &guid, sizeof(GUID)) == 0)
		{
			*format = ImageFormat::WMF;
		}

		return status;
	}

	DLL_EXPORT UINT BitmapToFile(size_t bitmap, LPCTSTR path, LPCTSTR gdiplusFormat, UINT quality)
	{
		if (bitmap == 0)
		{
			return Status::InvalidParameter;
		}

		Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
		CLSID imgClsId;
		int encoderIndex = GetImageEncoderClsid(gdiplusFormat, &imgClsId);
		if (encoderIndex < 0)
		{
			return Status::InvalidParameter;
		}

		Gdiplus::EncoderParameters encoderParams;
		Gdiplus::EncoderParameters* pEncoderParams = nullptr;

		if (wcscmp(gdiplusFormat, L"image/jpeg") == 0)
		{
			encoderParams.Count = 1;
			encoderParams.Parameter[0].Guid = Gdiplus::EncoderQuality;
			encoderParams.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
			encoderParams.Parameter[0].NumberOfValues = 1;
			encoderParams.Parameter[0].Value = &quality;
			pEncoderParams = &encoderParams;
		}

		Status status = pBitmap->Save(path, &imgClsId, pEncoderParams);
		return status;
	}

	DLL_EXPORT UINT BitmapToStream(size_t bitmap, size_t* outStream, LPCTSTR gdiplusFormat, UINT quality)
	{
		if (bitmap == 0 || outStream == nullptr)
		{
			return Status::InvalidParameter;
		}

		Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
		CLSID imgClsId;
		int encoderIndex = GetImageEncoderClsid(gdiplusFormat, &imgClsId);
		if (encoderIndex < 0)
		{
			return Status::InvalidParameter;
		}

		Gdiplus::EncoderParameters encoderParams;
		Gdiplus::EncoderParameters* pEncoderParams = nullptr;

		if (wcscmp(gdiplusFormat, L"image/jpeg") == 0)
		{
			encoderParams.Count = 1;
			encoderParams.Parameter[0].Guid = Gdiplus::EncoderQuality;
			encoderParams.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
			encoderParams.Parameter[0].NumberOfValues = 1;
			encoderParams.Parameter[0].Value = &quality;
			pEncoderParams = &encoderParams;
		}

		IStream* pStream = nullptr;
		HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &pStream);
		if (pStream == nullptr)
		{
			return Status::OutOfMemory;
		}

		Status status = pBitmap->Save(pStream, &imgClsId, pEncoderParams);
		if (status != Status::Ok)
		{
			pStream->Release();
			return status;
		}

		*outStream = reinterpret_cast<size_t>(pStream);
		return status;
	}

	DLL_EXPORT void BitmapRelease(size_t bitmap)
	{
		if (bitmap)
		{
			Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
			delete pBitmap;
		}
	}

	DLL_EXPORT uint64_t BitmapGetWidthAndHeight(size_t bitmap)
	{
		if (bitmap)
		{
			Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
			return pBitmap->GetWidth() | (static_cast<uint64_t>(pBitmap->GetHeight()) << 32);
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

	DLL_EXPORT UINT BitmapSetPixel(size_t bitmap, int x, int y, UINT argb)
	{
		if (bitmap == 0)
		{
			return Status::InvalidParameter;
		}

		Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
		Color color(argb);
		Status status = pBitmap->SetPixel(x, y, color);
		return status;
	}

	DLL_EXPORT UINT BitmapGetPixelsHorizontally(size_t bitmap, int x, int y, UINT* array, int size)
	{
		if (bitmap == 0 || array == 0 || size <= 0)
		{
			return Status::InvalidParameter;
		}

		Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
		int width = static_cast<int>(pBitmap->GetWidth());
		Gdiplus::Rect rect{ x, y, size, 1 };
		if (x + size > width)
		{
			rect.X = 0;
			rect.Width = width;
			rect.Height += (size + x - 1) / width;
		}
		Gdiplus::BitmapData bmpData{};
		Status status = pBitmap->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeRead, PixelFormat32bppARGB, &bmpData);
		memcpy(array, x + size > width ? (char*)bmpData.Scan0 + x * 4 : (char*)bmpData.Scan0, size * 4);
		pBitmap->UnlockBits(&bmpData);
		return status;
	}

	DLL_EXPORT UINT BitmapSetPixelsHorizontally(size_t bitmap, int x, int y, UINT* array, int size)
	{
		if (bitmap == 0 || array == 0 || size <= 0)
		{
			return Status::InvalidParameter;
		}

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
		Status status = pBitmap->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeRead, PixelFormat32bppARGB, &bmpData);
		memcpy(x + size > width ? (char*)bmpData.Scan0 + x * 4 : (char*)bmpData.Scan0, array, size * 4);
		pBitmap->UnlockBits(&bmpData);
		return status;
	}

	DLL_EXPORT UINT BitmapGetPixelsVertically(size_t bitmap, int x, int y, UINT* array, int size)
	{
		if (bitmap == 0 || array == 0 || size <= 0)
		{
			return Status::InvalidParameter;
		}

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

		return Status::Ok;
	}

	DLL_EXPORT UINT BitmapSetPixelsVertically(size_t bitmap, int x, int y, UINT* array, int size)
	{
		if (bitmap == 0 || array == 0 || size <= 0)
		{
			return Status::InvalidParameter;
		}

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

		return Status::Ok;
	}

	DLL_EXPORT UINT BitmapGetPixelsOfRect(size_t bitmap, int x, int y, int width, int height, UINT* array)
	{
		if (bitmap == 0 || array == 0)
		{
			return Status::InvalidParameter;
		}

		Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
		Gdiplus::Rect rect(x, y, width, height);
		Gdiplus::BitmapData bmpData{};
		bmpData.Width = rect.Width;
		bmpData.Height = rect.Height;
		bmpData.PixelFormat = PixelFormat32bppARGB;
		bmpData.Stride = rect.Width * 4;
		bmpData.Scan0 = array;
		Status status = pBitmap->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeUserInputBuf | Gdiplus::ImageLockMode::ImageLockModeRead,
			PixelFormat32bppARGB, &bmpData);
		pBitmap->UnlockBits(&bmpData);
		return status;
	}

	DLL_EXPORT UINT BitmapSetPixelsOfRect(size_t bitmap, int x, int y, int width, int height, UINT* array)
	{
		if (bitmap == 0 || array == 0)
		{
			return Status::InvalidParameter;
		}

		Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
		Gdiplus::Rect rect(x, y, width, height);
		Gdiplus::BitmapData bmpData{};
		bmpData.Width = rect.Width;
		bmpData.Height = rect.Height;
		bmpData.PixelFormat = PixelFormat32bppARGB;
		bmpData.Stride = rect.Width * 4;
		bmpData.Scan0 = array;
		Status status = pBitmap->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeUserInputBuf | Gdiplus::ImageLockMode::ImageLockModeWrite,
			PixelFormat32bppARGB, &bmpData);
		pBitmap->UnlockBits(&bmpData);
		return status;
	}


	DLL_EXPORT UINT MultiBitmapGetFrameCount(size_t bitmap)
	{
		if (bitmap == 0)
		{
			return 0;
		}

		UINT format = 0;
		BitmapGetRawFormat(bitmap, &format);

		Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
		if (format == ImageFormat::GIF)
		{
			return pBitmap->GetFrameCount(&Gdiplus::FrameDimensionTime);
		}

		return pBitmap->GetFrameCount(&Gdiplus::FrameDimensionPage);
	}

	DLL_EXPORT UINT MultiBitmapGetFrameDelaySize(size_t bitmap)
	{
		if (bitmap == 0)
		{
			return 0;
		}

		Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
		return pBitmap->GetPropertyItemSize(PropertyTagFrameDelay);
	}

	DLL_EXPORT UINT MultiBitmapGetFrameDelay(size_t bitmap, char* delay, UINT size, int *valueOffset)
	{
		if (bitmap == 0)
		{
			return Status::InvalidParameter;
		}

		Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
		PropertyItem* pPropertyItem = reinterpret_cast<PropertyItem*>(delay);
		Status status = pBitmap->GetPropertyItem(PropertyTagFrameDelay, size, pPropertyItem);
		if (valueOffset)
		{
			*valueOffset = static_cast<int>(reinterpret_cast<char*>(pPropertyItem->value) - reinterpret_cast<char*>(pPropertyItem));
		}
		return status;
	}

	DLL_EXPORT UINT MultiBitmapSelectActiveFrame(size_t bitmap, UINT frameIndex)
	{
		if (bitmap == 0)
		{
			return Status::InvalidParameter;
		}

		UINT format = 0;
		BitmapGetRawFormat(bitmap, &format);

		Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
		Status status = Status::Ok;
		if (format == ImageFormat::GIF)
		{
			status = pBitmap->SelectActiveFrame(&Gdiplus::FrameDimensionTime, frameIndex);
		}
		else
		{
			status = pBitmap->SelectActiveFrame(&Gdiplus::FrameDimensionPage, frameIndex);
		}
		return status;
	}

	DLL_EXPORT UINT MultiBitmapToFile(size_t* bitmaps, ULONG* delay, UINT size, LPCTSTR path, LPCTSTR format)
	{
		// https://learn.microsoft.com/en-us/windows/win32/gdiplus/-gdiplus-creating-and-saving-a-multiple-frame-image-use
		// not support generate gif file
		if (path == nullptr || format == nullptr || bitmaps == 0)
		{
			return Status::InvalidParameter;
		}

		CLSID encoderClsid;
		if (GetImageEncoderClsid(format, &encoderClsid) < 0)
		{
			return Status::GdiplusNotInitialized;
		}

		PropertyItem propItem = {};
		propItem.id = PropertyTagFrameDelay;
		propItem.type = PropertyTagTypeLong;
		propItem.length = size * sizeof(ULONG);
		propItem.value = delay;

		EncoderParameters encoderParams;
		encoderParams.Count = 1;
		encoderParams.Parameter[0].Guid = EncoderSaveFlag;
		encoderParams.Parameter[0].Type = EncoderParameterValueTypeLong;
		encoderParams.Parameter[0].NumberOfValues = 1;
		ULONG parameterValue = EncoderValueMultiFrame;
		encoderParams.Parameter[0].Value = &parameterValue;

		Bitmap* pFirst = reinterpret_cast<Bitmap*>(bitmaps[0]);
		Status status = Status::Ok;
   //     if (wcscmp(format, L"image/gif") == 0)
   //     {
			//status = pFirst->SetPropertyItem(&propItem);
   //     }
		status = pFirst->Save(path, &encoderClsid, &encoderParams);
		if (status != Status::Ok)
		{
			return status;
		}


		for (UINT n = 1; n < size; ++n)
		{
			Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmaps[n]);
			parameterValue = EncoderValueFrameDimensionPage;
			status = pFirst->SaveAdd(pBitmap, &encoderParams);
			if (status != Status::Ok)
			{
				return status;
			}
		}

		parameterValue = EncoderValueFlush;
		status = pFirst->SaveAdd(&encoderParams);
		return status;
	}

	DLL_EXPORT UINT GetStreamSize(size_t stream)
	{
		if (stream == 0)
		{
			return 0;
		}

		IStream* pStream = reinterpret_cast<IStream*>(stream);
		HGLOBAL hGlobal = nullptr;
		UINT size = 0;
		HRESULT hr = GetHGlobalFromStream(pStream, &hGlobal);
		if (SUCCEEDED(hr))
		{
			size = static_cast<UINT>(GlobalSize(hGlobal));
		}

		return size;
	}

	DLL_EXPORT UINT CopyStreamData(size_t stream, char* outData, UINT outSize)
	{
		if (stream == 0 || outData == nullptr)
		{
			return 0;
		}

		IStream* pStream = reinterpret_cast<IStream*>(stream);
		HGLOBAL hGlobal = nullptr;
		void* pData = nullptr;
		UINT size = 0;
		HRESULT hr = GetHGlobalFromStream(pStream, &hGlobal);
		if (SUCCEEDED(hr))
		{
			pData = GlobalLock(hGlobal);
			if (pData)
			{
				size = static_cast<UINT>(GlobalSize(hGlobal));
				memcpy(outData, pData, min(outSize, size));
				GlobalUnlock(hGlobal);
			}
		}

		return size;
	}

	DLL_EXPORT void ReleaseStream(size_t stream)
	{
		if (stream == 0)
		{
			return;
		}

		IStream* pStream = reinterpret_cast<IStream*>(stream);
		pStream->Release();
	}

	DLL_EXPORT UINT SaveGif(size_t* bitmaps, UINT* delay, UINT size, LPCTSTR path)
	{
		if (path == nullptr || bitmaps == nullptr || delay == nullptr || size == 0)
		{
			return Status::InvalidParameter;
		}

		Bitmap* pFirst = reinterpret_cast<Bitmap*>(bitmaps[0]);
		Status status = Status::Ok;
		UINT width = pFirst->GetWidth();
		UINT height = pFirst->GetHeight();

		GifWriter gw;
		GifBegin(&gw, path, width, height, delay[0]);
		for (UINT n = 0; n < size; ++n)
		{
			Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmaps[n]);
			width = pFirst->GetWidth();
			height = pFirst->GetHeight();
			Gdiplus::Rect rect{ 0, 0, static_cast<int>(width), static_cast<int>(height) };
			Gdiplus::BitmapData bmpData{};
			//bmpData.PixelFormat = PixelFormat32bppARGB;
			//bmpData.Stride = rect.Width * 4;
			status = pBitmap->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeRead, PixelFormat32bppARGB, &bmpData);
			if (status == Status::Ok)
			{
				uint8_t* pBmpData = reinterpret_cast<uint8_t*>(bmpData.Scan0);
				uint8_t temp = 0;
				for (UINT n = 0, bmsize = width * height * 4; n < bmsize; n+=4)
				{
					// gdi+ PixelFormat32bppARGB 0xAARRGGBB, byte order is b g r a
					// to byte order r g b a
					temp = pBmpData[n];
					pBmpData[n] = pBmpData[n + 2];
					pBmpData[n + 2] = temp;
				}
				GifWriteFrame(&gw, pBmpData, width, height, delay[n]);
				pBitmap->UnlockBits(&bmpData);
			}
		}

		return GifEnd(&gw) ? Status::Ok : Status::GenericError;
	}

#ifdef __cplusplus
}
#endif
