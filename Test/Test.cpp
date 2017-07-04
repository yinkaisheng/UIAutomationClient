// Test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include <Locale.h>
#include "../src/UIAutomationClient.h"

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "chs");
	if (InitInstance())
	{
		size_t root = GetRootElement();
		size_t name = GetElementName(root);
		BSTR bName = (BSTR)name;
        WCHAR *pszName = (WCHAR*)bName;
		wprintf(L"root control: %s\n", pszName);
		FreeBSTR(name);
		ReleaseElement(root);

		size_t cmd = GetProcessCommandLine(6212);
		DeleteWCharArray(cmd);

		size_t bitmap = BitmapCreate(500, 500);
		DWORD start = GetTickCount();
		for (int x=0; x<500; ++x)
		{
			for (int y=0; y<500; ++y)
			{
				BitmapSetPixel(bitmap, x, y, 0x0000FF|(255*x/500<<24));
			}
		}
		wprintf(L"write 500x500 image cost %ums\n", GetTickCount()-start);

		start = GetTickCount();
		for (int x=0; x<500; ++x)
		{
			for (int y=0; y<500; ++y)
			{
				BitmapGetPixel(bitmap, x, y);
			}
		}
		wprintf(L"read 500x500 image cost %ums\n", GetTickCount()-start);

		ReleaseInstance();
	}
	else
	{
		wprintf(L"InitInstance failed\n");
	}

	getchar();
	return 0;
}

