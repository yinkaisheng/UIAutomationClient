// Test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include <vector>
#include <memory>
#include "../src/UIAutomationClient.h"

void TestCreateBitmap();
void TestLoadGif();
void TestGenerateTif();
void TestRotate();

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD pid = GetCurrentProcessId();
#ifdef _DEBUG
	//DWORD ppid = GetParentProcessId(6656);
#endif
    int mCount = GetSystemMetrics(SM_CMONITORS);
    std::vector<int> rects(mCount*4, 0);
    UINT count = GetMonitorsRect(&rects[0], (int)rects.size(), 0);
    printf("dpi aware = 0\n");
    for (int val : rects)
    {
        printf("%d\n", val);
    }

    printf("\ndpi aware = 2\n");
    count = GetMonitorsRect(&rects[0], (int)rects.size(), 2);
    for (int val : rects)
    {
        printf("%d\n", val);
    }

    //getchar();
    //return 0;

	Initialize();

	GetImageEncoderClsid(L"invalid test", nullptr);
	//TestGif();
	TestGenerateTif();
	//TestRotate();

	Uninitialize();
	getchar();
	return 0;
}

void TestCreateBitmap()
{
	size_t bitmap = BitmapCreate(500, 500);

	for (int x = 0; x < 500; ++x)
	{
		for (int y = 0; y < 500; ++y)
		{
			BitmapSetPixel(bitmap, x, y, 0x0000FF | (255 * x / 500 << 24));
		}
	}

	for (int x = 0; x < 500; ++x)
	{
		for (int y = 0; y < 500; ++y)
		{
			BitmapGetPixel(bitmap, x, y);
		}
	}

	UINT array[100] = { 0 };
	BitmapGetPixelsOfRect(bitmap, 0, 0, 10, 10, array);

	unsigned char a, r, g, b;
	for (int i = 0; i < 100; ++i)
	{
		a = (array[i] >> 24) & 0xFF;
		r = (array[i] >> 16) & 0xFF;
		g = (array[i] >> 8) & 0xFF;
		b = array[1] & 0xFF;
		array[i] = (a << 24) | (b << 16) | (g << 8) | r;
	}

	BitmapSetPixelsOfRect(bitmap, 0, 0, 10, 10, array);
}

void TestLoadGif()
{
	size_t gif = BitmapFromFile(LR"(F:\media\pictures\testgif.gif)");
	UINT format = 0;
	BitmapGetRawFormat(gif, &format);
	printf("raw format %u\n", format);
	UINT count = MultiBitmapGetFrameCount(gif);
	printf("gif frame count %u\n", count);
	UINT delaySize = MultiBitmapGetFrameDelaySize(gif);
	printf("gif delaySize %u\n", delaySize);
	std::unique_ptr<char, decltype(&free)> delay((char*)malloc(delaySize), free);
	UINT status = MultiBitmapGetFrameDelay(gif, delay.get(), delaySize, nullptr);
	printf("GifGetFrameDelay status %u\n", status);
	status = BitmapToFile(gif, LR"(F:\media\pictures\testgif1.gif)", L"image/gif");
	status = MultiBitmapSelectActiveFrame(gif, 2);
	status = BitmapToFile(gif, LR"(F:\media\pictures\testgif3.gif)", L"image/gif");
	BitmapRelease(gif);
}

void TestGenerateTif()
{
	size_t bitmap = BitmapFromFile(LR"(F:\media\pictures\testgif.gif)");
	uint64_t size = BitmapGetWidthAndHeight(bitmap);
	UINT width = size & 0xFFFFFFFF;
	UINT height = size >> 32;
	size_t bitmap2 = BitmapRotateWithSameSize(bitmap, width / 2, height / 2, 45.0);

	size_t bmps[2] = { bitmap, bitmap2 };
	UINT status = MultiBitmapToFile(bmps, nullptr, 2, L"test.tif", L"image/tiff");

	BitmapRelease(bitmap);
	BitmapRelease(bitmap2);
}

void TestRotate()
{
	size_t bitmap = BitmapFromFile(LR"(F:\media\pictures\360block.png)");
	uint64_t size = BitmapGetWidthAndHeight(bitmap);
	UINT width = size & 0xFFFFFFFF;
	UINT height = size >> 32;
	//size_t bitmapR = BitmapRotateFrom(bitmap, 45);
	size_t bitmapR = BitmapRotateWithSameSize(bitmap, width, height, 45);
	BitmapToFile(bitmapR, LR"(F:\media\pictures\360block45.png)", L"image/png");
	BitmapRelease(bitmapR);
	BitmapRelease(bitmap);
}

