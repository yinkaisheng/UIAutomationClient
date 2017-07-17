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
        int processId = GetCurrentProcessId();
        TCHAR szText[MAX_PATH] = { 0 };
        GetProcessCommandLine(processId, szText, MAX_PATH);
        wprintf(L"ProcessCommandLine: %s\n\n", szText);

		size_t root = GetRootElement();
        size_t name = GetElementClassName(root);
        wprintf(L"ClassName: %s", (WCHAR*)(BSTR)name);
        FreeBSTR(name);

        name = GetElementName(root);
        wprintf(L"    Name: %s\n", (WCHAR*)(BSTR)name);
        FreeBSTR(name);

        size_t current = GetFirstChildElement(root);
        size_t next = current;
        while (current)
        {
            name = GetElementClassName(current);
            wprintf(L"    ClassName: %s", (WCHAR*)(BSTR)name);
            FreeBSTR(name);

            name = GetElementName(current);
            wprintf(L"    Name: %s\n", (WCHAR*)(BSTR)name);
            FreeBSTR(name);

            next = GetNextSiblingElement(current);
            ReleaseElement(current);
            current = next;
        }

		ReleaseElement(root);

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

