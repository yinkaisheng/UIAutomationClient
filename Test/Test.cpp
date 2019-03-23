// Test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include "../src/UIAutomationClient.h"

int _tmain(int argc, _TCHAR* argv[])
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

	getchar();
	return 0;
}

