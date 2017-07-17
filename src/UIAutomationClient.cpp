//Author: yinkaisheng@foxmail.com
#include "stdafx.h"
#include <tchar.h>
#include <stdio.h>
#include <Winternl.h>
#include <UIAutomation.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

#define __EXPORT_DLL__
#include "UIAutomationClient.h"

#if (_MSC_VER <= 1500)
#define nullptr 0
#endif

IUIAutomation *g_pAutomation = nullptr;
IUIAutomationTreeWalker *g_pRawTreeWalker = nullptr;
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
            CoInitialize(nullptr);
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
            ReleaseInstance();
            CoUninitialize();
            //printf("AutomationClient: CoUninitialize CUIAutomation.\n");
            break;
        }
    }
    return TRUE;
}

#ifdef __cplusplus
extern "C"
{
#endif

    DLL_EXPORT BOOL InitInstance()
    {
        if (!g_pAutomation)
        {
            HRESULT hr = CoCreateInstance(__uuidof(CUIAutomation), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation), reinterpret_cast<void**>(&g_pAutomation));
            //printf("AutomationClient: CoCreateInstance CUIAutomation %s.\n", SUCCEEDED(hr) ? "succeed" : "failed");
        }
        if (!g_pRawTreeWalker && g_pAutomation)
        {
            g_pAutomation->get_RawViewWalker(&g_pRawTreeWalker);
        }
        if (!g_pAutomation || !g_pRawTreeWalker)
        {
            return FALSE;
        }
        if (!g_nGdiPlusToken)
        {
            GdiplusStartupInput gdiplusStartupInput;
            GdiplusStartup(&g_nGdiPlusToken, &gdiplusStartupInput, NULL);
        }
        return TRUE;
    }

    DLL_EXPORT void ReleaseInstance()
    {
        if (g_pAutomation)
        {
            g_pAutomation->Release();
            g_pAutomation = nullptr;
        }
        if (g_pRawTreeWalker)
        {
            g_pRawTreeWalker->Release();
            g_pRawTreeWalker = nullptr;
        }
        if (g_nGdiPlusToken)
        {
            GdiplusShutdown(g_nGdiPlusToken);
            g_nGdiPlusToken = 0;
        }
    }

    DLL_EXPORT int WcsCpy(wchar_t *dest, int size, const wchar_t* src)
    {
        return wcscpy_s(dest, size, src);
    }

    DLL_EXPORT UINT SendUnicodeChar(wchar_t *pwChar)
    {
        INPUT u_input[2];

        u_input[0].type = INPUT_KEYBOARD;
        u_input[0].ki.wVk = 0;
        u_input[0].ki.wScan = *pwChar;
        u_input[0].ki.dwFlags = KEYEVENTF_UNICODE;
        u_input[0].ki.time = 0;
        // L25: Set dwExtraInfo to ensure AutoHotkey ignores the event; otherwise it may trigger a SCxxx hotkey (where xxx is u_code).
        u_input[0].ki.dwExtraInfo = 0;

        u_input[1].type = INPUT_KEYBOARD;
        u_input[1].ki.wVk = 0;
        u_input[1].ki.wScan = *pwChar;
        u_input[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        u_input[1].ki.time = 0;
        u_input[1].ki.dwExtraInfo = 0;

        return SendInput(2, u_input, sizeof(INPUT));
    }

    // #include <window.h> <Winternl.h> first, this way only can get current process
    DLL_EXPORT int GetParentProcessId(int nProcessId)
    {
        HANDLE hProcess = nullptr;

        if (-1 == nProcessId)
        {
            hProcess = GetCurrentProcess();
        }
        else
        {
            hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, nProcessId);
        }

        if (hProcess)
        {
            typedef NTSTATUS (WINAPI *pfnNtQueryInformationProcess)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
            HMODULE hMod = GetModuleHandle(TEXT("ntdll.dll"));
            pfnNtQueryInformationProcess ntQueryInformationProcess = 
                reinterpret_cast<pfnNtQueryInformationProcess>(GetProcAddress(hMod, "NtQueryInformationProcess"));
            PROCESS_BASIC_INFORMATION stInfo = {0};
            DWORD dwRetnLen = 0;
            NTSTATUS nRet = ntQueryInformationProcess(hProcess, ProcessBasicInformation, &stInfo, sizeof(stInfo), &dwRetnLen);
            CloseHandle(hProcess);

            if (0 == nRet)
            {
                return static_cast<int>(reinterpret_cast<size_t>(stInfo.Reserved3));
            }
        }

        return -1;
    }

    DLL_EXPORT BOOL GetProcessCommandLine(UINT nProcessId, wchar_t* pOut, int size)
    {
        if (pOut && size > 0)
        {
            HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, nProcessId);

            if (!hProcess)
            {
                return FALSE;
            }

            DWORD dwThreadId = 0;
            DWORD dwExitCode = 0;
            SIZE_T dwReadedBytes = 0;
            HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&GetCommandLine), nullptr, 0, &dwThreadId);

            if (hThread)
            {
                WaitForSingleObject(hThread, INFINITE);
                GetExitCodeThread(hThread, &dwExitCode);
                return ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(static_cast<size_t>(dwExitCode)), pOut, size * sizeof(wchar_t), &dwReadedBytes); // param 4, max bytes
            }
        }
        return FALSE;
    }

    DLL_EXPORT void FreeBSTR(size_t bstr)
    {
        BSTR bstrStr = reinterpret_cast<BSTR>(bstr);
        if (bstrStr)
        {
            SysFreeString(bstrStr);
        }
    }

    DLL_EXPORT size_t GetAutomationObject()
    {
        return reinterpret_cast<size_t>(g_pAutomation);
    }

    DLL_EXPORT size_t GetRawTreeWalker()
    {
        return reinterpret_cast<size_t>(g_pRawTreeWalker);
    }

    DLL_EXPORT ULONG ElementAddRef(size_t element)
    {
        IUIAutomationElement* pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (pElement)
        {
            return pElement->AddRef();
        }
        return 0;
    }

    DLL_EXPORT BOOL CompareElements(size_t element1, size_t element2)
    {
        BOOL areSame = FALSE;
        if (g_pAutomation)
        {
            IUIAutomationElement* pElement1 = reinterpret_cast<IUIAutomationElement*>(element1);
            IUIAutomationElement* pElement2 = reinterpret_cast<IUIAutomationElement*>(element2);
            g_pAutomation->CompareElements(pElement1, pElement2, &areSame);
        }
        return areSame;
    }

    DLL_EXPORT ULONG ReleaseElement(size_t element)
    {
        IUIAutomationElement* pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (pElement)
        {
            return pElement->Release();
        }
        return 0;
    }

    DLL_EXPORT ULONG ReleaseElementArray(size_t elementArray)
    {
        IUIAutomationElementArray* pElementArray = reinterpret_cast<IUIAutomationElementArray*>(elementArray);
        if (pElementArray)
        {
            return pElementArray->Release();
        }
        return 0;
    }

    DLL_EXPORT size_t GetRootElement()
    {
        IUIAutomationElement* pElement = nullptr;
        if (g_pAutomation)
        {
            g_pAutomation->GetRootElement(&pElement);
        }
        return reinterpret_cast<size_t>(pElement);
    }

    DLL_EXPORT size_t GetFocusedElement()
    {
        IUIAutomationElement* pElement = nullptr;
        if (g_pAutomation)
        {
            g_pAutomation->GetFocusedElement(&pElement);
        }
        return reinterpret_cast<size_t>(pElement);
    }

    DLL_EXPORT size_t ElementFromPoint(int x, int y)
    {
        IUIAutomationElement* pElement = nullptr;
        if (g_pAutomation)
        {
            POINT pt;
            pt.x = x;
            pt.y = y;
            g_pAutomation->ElementFromPoint(pt, &pElement);
        }
        return reinterpret_cast<size_t>(pElement);
    }

    DLL_EXPORT size_t ElementFromHandle(size_t handle)
    {
        IUIAutomationElement* pElement = nullptr;
        if (g_pAutomation)
        {
            g_pAutomation->ElementFromHandle(reinterpret_cast<UIA_HWND>(handle), &pElement);
        }
        return reinterpret_cast<size_t>(pElement);
    }

    DLL_EXPORT size_t GetElementName(size_t element)
    {
        BSTR name = nullptr;
        IUIAutomationElement* pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (pElement)
        {
            pElement->get_CurrentName(&name);
        }
        return reinterpret_cast<size_t>(name);
    }

    DLL_EXPORT size_t GetElementControlTypeName(size_t element)
    {
        BSTR name = nullptr;
        IUIAutomationElement* pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (pElement)
        {
            pElement->get_CurrentLocalizedControlType(&name);
        }
        return reinterpret_cast<size_t>(name);
    }

    DLL_EXPORT size_t GetElementClassName(size_t element)
    {
        BSTR name = nullptr;
        IUIAutomationElement* pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (pElement)
        {
            pElement->get_CurrentClassName(&name);
        }
        return reinterpret_cast<size_t>(name);
    }

    DLL_EXPORT size_t GetElementAutomationId(size_t element)
    {
        BSTR name = nullptr;
        IUIAutomationElement* pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (pElement)
        {
            pElement->get_CurrentAutomationId(&name);
        }
        return reinterpret_cast<size_t>(name);
    }

    DLL_EXPORT int GetElementProcessId(size_t element)
    {
        int processId = 0;
        IUIAutomationElement* pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (pElement)
        {
            pElement->get_CurrentProcessId(&processId);
        }
        return processId;
    }

    DLL_EXPORT int GetElementControlType(size_t element)
    {
        IUIAutomationElement* pElement = reinterpret_cast<IUIAutomationElement*>(element);
        CONTROLTYPEID controlType = 0;
        if (pElement)
        {
            pElement->get_CurrentControlType(&controlType);
        }
        return controlType;
    }

    DLL_EXPORT size_t GetElementLocalizedControlType(size_t element)
    {
        BSTR name = nullptr;
        IUIAutomationElement* pElement = reinterpret_cast<IUIAutomationElement*>(element);
        CONTROLTYPEID controlType = 0;
        if (pElement)
        {
            pElement->get_CurrentLocalizedControlType(&name);
        }
        return reinterpret_cast<size_t>(name);;
    }

    DLL_EXPORT HRESULT GetElementBoundingRectangle(size_t element, RECT* pRect)
    {
        IUIAutomationElement* pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (pElement)
        {
            return pElement->get_CurrentBoundingRectangle(pRect);
        }
        return S_FALSE;
    }

    DLL_EXPORT BOOL GetElementIsEnabled(size_t element)
    {
        BOOL isEnable = FALSE;
        IUIAutomationElement* pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (pElement)
        {
            pElement->get_CurrentIsEnabled(&isEnable);
        }
        return isEnable;
    }

    DLL_EXPORT BOOL GetElementHasKeyboardFocus(size_t element)
    {
        BOOL hasKeyboardFocus = FALSE;
        IUIAutomationElement* pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (pElement)
        {
            pElement->get_CurrentHasKeyboardFocus(&hasKeyboardFocus);
        }
        return hasKeyboardFocus;
    }

    DLL_EXPORT BOOL GetElementIsKeyboardFocusable(size_t element)
    {
        BOOL isKeyboardFocusable = FALSE;
        IUIAutomationElement* pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (pElement)
        {
            pElement->get_CurrentIsKeyboardFocusable(&isKeyboardFocusable);
        }
        return isKeyboardFocusable;
    }

    DLL_EXPORT BOOL GetElementIsOffscreen(size_t element)
    {
        BOOL isOffscreen = FALSE;
        IUIAutomationElement* pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (pElement)
        {
            pElement->get_CurrentIsOffscreen(&isOffscreen);
        }
        return isOffscreen;
    }

    DLL_EXPORT size_t GetElementHandle(size_t element)
    {
        UIA_HWND hwnd = 0;
        IUIAutomationElement* pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (pElement)
        {
            pElement->get_CurrentNativeWindowHandle(&hwnd);
        }
        return reinterpret_cast<size_t>(hwnd);
    }

    DLL_EXPORT HRESULT SetElementFocus(size_t element)
    {
        IUIAutomationElement* pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (pElement)
        {
            return pElement->SetFocus();
        }
        return S_FALSE;
    }

    DLL_EXPORT size_t GetParentElement(size_t element)
    {
        IUIAutomationElement *pParent = nullptr;
        IUIAutomationElement *pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (g_pRawTreeWalker && pElement)
        {
            g_pRawTreeWalker->GetParentElement(pElement, &pParent);
        }
        return reinterpret_cast<size_t>(pParent);
    }

    DLL_EXPORT size_t GetNextSiblingElement(size_t element)
    {
        IUIAutomationElement *pNext = nullptr;
        IUIAutomationElement *pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (g_pRawTreeWalker && pElement)
        {
            g_pRawTreeWalker->GetNextSiblingElement(pElement, &pNext);
        }
        return reinterpret_cast<size_t>(pNext);
    }

    DLL_EXPORT size_t GetPreviousSiblingElement(size_t element)
    {
        IUIAutomationElement *pPrevious = nullptr;
        IUIAutomationElement *pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (g_pRawTreeWalker && pElement)
        {
            g_pRawTreeWalker->GetPreviousSiblingElement(pElement, &pPrevious);
        }
        return reinterpret_cast<size_t>(pPrevious);
    }

    DLL_EXPORT size_t GetFirstChildElement(size_t element)
    {
        IUIAutomationElement *pChild = nullptr;
        IUIAutomationElement *pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (g_pRawTreeWalker && pElement)
        {
            g_pRawTreeWalker->GetFirstChildElement(pElement, &pChild);
        }
        return reinterpret_cast<size_t>(pChild);
    }

    DLL_EXPORT size_t GetLastChildElement(size_t element)
    {
        IUIAutomationElement *pChild = nullptr;
        IUIAutomationElement *pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (g_pRawTreeWalker && pElement)
        {
            g_pRawTreeWalker->GetLastChildElement(pElement, &pChild);
        }
        return reinterpret_cast<size_t>(pChild);
    }

    DLL_EXPORT int ElementArrayGetLength(size_t elementArray)
    {
        int length = 0;
        IUIAutomationElementArray *pArray = reinterpret_cast<IUIAutomationElementArray*>(elementArray);
        if (pArray)
        {
            pArray->get_Length(&length);
        }
        return length;
    }

    DLL_EXPORT size_t ElementArrayGetElement(size_t elementArray, int index)
    {
        IUIAutomationElement *pElement = nullptr;
        IUIAutomationElementArray *pArray = reinterpret_cast<IUIAutomationElementArray*>(elementArray);
        if (pArray)
        {
            pArray->GetElement(index, &pElement);
        }
        return reinterpret_cast<size_t>(pElement);
    }

    DLL_EXPORT size_t GetElementPattern(size_t element, int patternId)
    {
        IUnknown *pPattern = nullptr;
        IUIAutomationElement *pElement = reinterpret_cast<IUIAutomationElement*>(element);
        if (pElement)
        {
            pElement->GetCurrentPattern(patternId, &pPattern);
        }
        return reinterpret_cast<size_t>(pPattern);
    }

    DLL_EXPORT ULONG ReleasePattern(size_t pattern)
    {
        IUnknown *pPattern = reinterpret_cast<IUnknown*>(pattern);
        if (pPattern)
        {
            return pPattern->Release();
        }
        return 0;
    }

    DLL_EXPORT HRESULT InvokePatternInvoke(size_t pattern)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationInvokePattern *pPattern = static_cast<IUIAutomationInvokePattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->Invoke();
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT HRESULT TogglePatternToggle(size_t pattern)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationTogglePattern *pPattern = static_cast<IUIAutomationTogglePattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->Toggle();
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT int TogglePatternCurrentToggleState(size_t pattern)
    {
        ToggleState state = ToggleState_Off;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationTogglePattern *pPattern = static_cast<IUIAutomationTogglePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentToggleState(&state);
            }
        }
        return static_cast<int>(state);
    }

    DLL_EXPORT HRESULT ExpandCollapsePatternExpand(size_t pattern)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationExpandCollapsePattern *pPattern = static_cast<IUIAutomationExpandCollapsePattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->Expand();
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT HRESULT ExpandCollapsePatternCollapse(size_t pattern)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationExpandCollapsePattern *pPattern = static_cast<IUIAutomationExpandCollapsePattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->Collapse();
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT int ExpandCollapsePatternCurrentExpandCollapseState(size_t pattern)
    {
        ExpandCollapseState expandCollapseState = ExpandCollapseState_Collapsed;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationExpandCollapsePattern *pPattern = static_cast<IUIAutomationExpandCollapsePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentExpandCollapseState(&expandCollapseState);
            }
        }
        return static_cast<int>(expandCollapseState);
    }

    DLL_EXPORT size_t ValuePatternCurrentValue(size_t pattern)
    {
        BSTR value = nullptr;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationValuePattern *pPattern = static_cast<IUIAutomationValuePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentValue(&value);
            }
        }
        return reinterpret_cast<size_t>(value);
    }

    DLL_EXPORT HRESULT ValuePatternSetValue(size_t pattern, wchar_t* pwValue)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown && pwValue)
        {
            IUIAutomationValuePattern *pPattern = static_cast<IUIAutomationValuePattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->SetValue(pwValue);
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT BOOL ValuePatternCurrentIsReadOnly(size_t pattern)
    {
        BOOL readOnly = FALSE;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationValuePattern *pPattern = static_cast<IUIAutomationValuePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentIsReadOnly(&readOnly);
            }
        }
        return readOnly;
    }

    DLL_EXPORT HRESULT ScrollItemPatternScrollIntoView(size_t pattern)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationScrollItemPattern *pPattern = static_cast<IUIAutomationScrollItemPattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->ScrollIntoView();
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT BOOL ScrollPatternCurrentHorizontallyScrollable(size_t pattern)
    {
        BOOL scroll = FALSE;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationScrollPattern *pPattern = static_cast<IUIAutomationScrollPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentHorizontallyScrollable(&scroll);
            }
        }
        return scroll;
    }

    DLL_EXPORT int ScrollPatternCurrentHorizontalViewSize(size_t pattern)
    {
        double size = 0;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationScrollPattern *pPattern = static_cast<IUIAutomationScrollPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentHorizontalViewSize(&size);
            }
        }
        return static_cast<int>(size);
    }

    DLL_EXPORT int ScrollPatternCurrentHorizontalScrollPercent(size_t pattern)
    {
        double size = 0;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationScrollPattern *pPattern = static_cast<IUIAutomationScrollPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentHorizontalScrollPercent(&size);
            }
        }
        return static_cast<int>(size);
    }

    DLL_EXPORT BOOL ScrollPatternCurrentVerticallyScrollable(size_t pattern)
    {
        BOOL scroll = FALSE;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationScrollPattern *pPattern = static_cast<IUIAutomationScrollPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentVerticallyScrollable(&scroll);
            }
        }
        return scroll;
    }

    DLL_EXPORT int ScrollPatternCurrentVerticalViewSize(size_t pattern)
    {
        double size = 0;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationScrollPattern *pPattern = static_cast<IUIAutomationScrollPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentVerticalViewSize(&size);
            }
        }
        return static_cast<int>(size);
    }

    DLL_EXPORT int ScrollPatternCurrentVerticalScrollPercent(size_t pattern)
    {
        double size = 0;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationScrollPattern *pPattern = static_cast<IUIAutomationScrollPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentVerticalScrollPercent(&size);
            }
        }
        return static_cast<int>(size);
    }

    DLL_EXPORT HRESULT ScrollPatternSetScrollPercent(size_t pattern, int hPercent, int vPercent)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationScrollPattern *pPattern = static_cast<IUIAutomationScrollPattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->SetScrollPercent(hPercent, vPercent);
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT size_t SelectionPatternGetCurrentSelection(size_t pattern)
    {
        IUIAutomationElementArray *pElementArray = nullptr;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationSelectionPattern *pPattern = static_cast<IUIAutomationSelectionPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->GetCurrentSelection(&pElementArray);
            }
        }
        return reinterpret_cast<size_t>(pElementArray);
    }

    DLL_EXPORT HRESULT SelectionItemPatternSelect(size_t pattern)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationSelectionItemPattern *pPattern = static_cast<IUIAutomationSelectionItemPattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->Select();
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT HRESULT SelectionItemPatternAddToSelection(size_t pattern)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationSelectionItemPattern *pPattern = static_cast<IUIAutomationSelectionItemPattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->AddToSelection();
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT HRESULT SelectionItemPatternRemoveFromSelection(size_t pattern)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationSelectionItemPattern *pPattern = static_cast<IUIAutomationSelectionItemPattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->RemoveFromSelection();
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT BOOL SelectionItemPatternCurrentIsSelected(size_t pattern)
    {
        BOOL isSelect = FALSE;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationSelectionItemPattern *pPattern = static_cast<IUIAutomationSelectionItemPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentIsSelected(&isSelect);
            }
        }
        return isSelect;
    }

    DLL_EXPORT int RangeValuePatternCurrentValue(size_t pattern)
    {
        double value = 0;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationRangeValuePattern *pPattern = static_cast<IUIAutomationRangeValuePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentValue(&value);
            }
        }
        return static_cast<int>(value);
    }

    DLL_EXPORT HRESULT RangeValuePatternSetValue(size_t pattern, int value)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationRangeValuePattern *pPattern = static_cast<IUIAutomationRangeValuePattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->SetValue(value);
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT int RangeValuePatternCurrentMaximum(size_t pattern)
    {
        double value = 0;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationRangeValuePattern *pPattern = static_cast<IUIAutomationRangeValuePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentMaximum(&value);
            }
        }
        return static_cast<int>(value);
    }

    DLL_EXPORT int RangeValuePatternCurrentMinimum(size_t pattern)
    {
        double value = 0;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationRangeValuePattern *pPattern = static_cast<IUIAutomationRangeValuePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentMinimum(&value);
            }
        }
        return static_cast<int>(value);
    }

    DLL_EXPORT int WindowPatternCurrentWindowVisualState(size_t pattern)
    {
        WindowVisualState state = WindowVisualState_Normal;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationWindowPattern *pPattern = static_cast<IUIAutomationWindowPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentWindowVisualState(&state);
            }
        }
        return static_cast<int>(state);
    }

    DLL_EXPORT HRESULT WindowPatternSetWindowVisualState(size_t pattern, int state)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationWindowPattern *pPattern = static_cast<IUIAutomationWindowPattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->SetWindowVisualState(static_cast<WindowVisualState>(state));
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT BOOL WindowPatternCurrentCanMaximize(size_t pattern)
    {
        BOOL can = FALSE;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationWindowPattern *pPattern = static_cast<IUIAutomationWindowPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentCanMaximize(&can);
            }
        }
        return can;
    }

    DLL_EXPORT BOOL WindowPatternCurrentCanMinimize(size_t pattern)
    {
        BOOL can = FALSE;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationWindowPattern *pPattern = static_cast<IUIAutomationWindowPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentCanMinimize(&can);
            }
        }
        return can;
    }

    DLL_EXPORT BOOL WindowPatternCurrentIsModal(size_t pattern)
    {
        BOOL flag = FALSE;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationWindowPattern *pPattern = static_cast<IUIAutomationWindowPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentIsModal(&flag);
            }
        }
        return flag;
    }

    DLL_EXPORT BOOL WindowPatternCurrentIsTopmost(size_t pattern)
    {
        BOOL flag = FALSE;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationWindowPattern *pPattern = static_cast<IUIAutomationWindowPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentIsTopmost(&flag);
            }
        }
        return flag;
    }

    DLL_EXPORT HRESULT WindowPatternClose(size_t pattern)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationWindowPattern *pPattern = static_cast<IUIAutomationWindowPattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->Close();
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT HRESULT LegacyIAccessiblePatternSelect(size_t pattern, long flag)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationLegacyIAccessiblePattern *pPattern = static_cast<IUIAutomationLegacyIAccessiblePattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->Select(flag);
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT HRESULT LegacyIAccessiblePatternDoDefaultAction(size_t pattern)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationLegacyIAccessiblePattern *pPattern = static_cast<IUIAutomationLegacyIAccessiblePattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->DoDefaultAction();
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT HRESULT LegacyIAccessiblePatternSetValue(size_t pattern, wchar_t* pwValue)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown && pwValue)
        {
            IUIAutomationLegacyIAccessiblePattern *pPattern = static_cast<IUIAutomationLegacyIAccessiblePattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->SetValue(pwValue);
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT int LegacyIAccessiblePatternCurrentChildId(size_t pattern)
    {
        int childId = 0;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationLegacyIAccessiblePattern *pPattern = static_cast<IUIAutomationLegacyIAccessiblePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentChildId(&childId);
            }
        }
        return childId;
    }

    DLL_EXPORT size_t LegacyIAccessiblePatternCurrentName(size_t pattern)
    {
        BSTR name = nullptr;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationLegacyIAccessiblePattern *pPattern = static_cast<IUIAutomationLegacyIAccessiblePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentName(&name);
            }
        }
        return reinterpret_cast<size_t>(name);
    }

    DLL_EXPORT size_t LegacyIAccessiblePatternCurrentValue(size_t pattern)
    {
        BSTR value = nullptr;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationLegacyIAccessiblePattern *pPattern = static_cast<IUIAutomationLegacyIAccessiblePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentValue(&value);
            }
        }
        return reinterpret_cast<size_t>(value);
    }

    DLL_EXPORT size_t LegacyIAccessiblePatternCurrentDescription(size_t pattern)
    {
        BSTR value = nullptr;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationLegacyIAccessiblePattern *pPattern = static_cast<IUIAutomationLegacyIAccessiblePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentDescription(&value);
            }
        }
        return reinterpret_cast<size_t>(value);
    }

    DLL_EXPORT DWORD LegacyIAccessiblePatternCurrentRole(size_t pattern)
    {
        DWORD value = 0;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationLegacyIAccessiblePattern *pPattern = static_cast<IUIAutomationLegacyIAccessiblePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentRole(&value);
            }
        }
        return value;
    }

    DLL_EXPORT DWORD LegacyIAccessiblePatternCurrentState(size_t pattern)
    {
        DWORD value = 0;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationLegacyIAccessiblePattern *pPattern = static_cast<IUIAutomationLegacyIAccessiblePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentState(&value);
            }
        }
        return value;
    }

    DLL_EXPORT size_t LegacyIAccessiblePatternCurrentHelp(size_t pattern)
    {
        BSTR value = nullptr;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationLegacyIAccessiblePattern *pPattern = static_cast<IUIAutomationLegacyIAccessiblePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentHelp(&value);
            }
        }
        return reinterpret_cast<size_t>(value);
    }

    DLL_EXPORT size_t LegacyIAccessiblePatternCurrentKeyboardShortcut(size_t pattern)
    {
        BSTR value = nullptr;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationLegacyIAccessiblePattern *pPattern = static_cast<IUIAutomationLegacyIAccessiblePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentKeyboardShortcut(&value);
            }
        }
        return reinterpret_cast<size_t>(value);
    }

    DLL_EXPORT size_t LegacyIAccessiblePatternGetCurrentSelection(size_t pattern)
    {
        IUIAutomationElementArray *value = nullptr;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationLegacyIAccessiblePattern *pPattern = static_cast<IUIAutomationLegacyIAccessiblePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->GetCurrentSelection(&value);
            }
        }
        return reinterpret_cast<size_t>(value);
    }

    DLL_EXPORT size_t LegacyIAccessiblePatternCurrentDefaultAction(size_t pattern)
    {
        BSTR value = nullptr;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationLegacyIAccessiblePattern *pPattern = static_cast<IUIAutomationLegacyIAccessiblePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentDefaultAction(&value);
            }
        }
        return reinterpret_cast<size_t>(value);
    }

    DLL_EXPORT size_t GridPatternGetItem(size_t pattern, int row, int column)
    {
        IUIAutomationElement *value = nullptr;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationGridPattern *pPattern = static_cast<IUIAutomationGridPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->GetItem(row, column, &value);
            }
        }
        return reinterpret_cast<size_t>(value);
    }

    DLL_EXPORT int GridPatternCurrentRowCount(size_t pattern)
    {
        int value = 0;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationGridPattern *pPattern = static_cast<IUIAutomationGridPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentRowCount(&value);
            }
        }
        return value;
    }

    DLL_EXPORT int GridPatternCurrentColumnCount(size_t pattern)
    {
        int value = 0;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationGridPattern *pPattern = static_cast<IUIAutomationGridPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentColumnCount(&value);
            }
        }
        return value;
    }

    DLL_EXPORT size_t TablePatternCurrentRowHeaders(size_t pattern)
    {
        IUIAutomationElementArray *value = nullptr;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationTablePattern *pPattern = static_cast<IUIAutomationTablePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->GetCurrentRowHeaders(&value);
            }
        }
        return reinterpret_cast<size_t>(value);
    }

    DLL_EXPORT size_t TablePatternCurrentColumnHeaders(size_t pattern)
    {
        IUIAutomationElementArray *value = nullptr;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationTablePattern *pPattern = static_cast<IUIAutomationTablePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->GetCurrentColumnHeaders(&value);
            }
        }
        return reinterpret_cast<size_t>(value);
    }

    DLL_EXPORT int TablePatternCurrentRowOrColumnMajor(size_t pattern)
    {
        RowOrColumnMajor value = RowOrColumnMajor_RowMajor;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationTablePattern *pPattern = static_cast<IUIAutomationTablePattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentRowOrColumnMajor(&value);
            }
        }
        return static_cast<int>(value);
    }

    DLL_EXPORT size_t TableItemPatternCurrentRowHeaderItems(size_t pattern)
    {
        IUIAutomationElementArray *value = nullptr;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationTableItemPattern *pPattern = static_cast<IUIAutomationTableItemPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->GetCurrentRowHeaderItems(&value);
            }
        }
        return reinterpret_cast<size_t>(value);
    }

    DLL_EXPORT size_t TableItemPatternCurrentColumnHeaderItems(size_t pattern)
    {
        IUIAutomationElementArray *value = nullptr;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationTableItemPattern *pPattern = static_cast<IUIAutomationTableItemPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->GetCurrentColumnHeaderItems(&value);
            }
        }
        return reinterpret_cast<size_t>(value);
    }

    DLL_EXPORT size_t GridItemPatternCurrentContainingGrid(size_t pattern)
    {
        IUIAutomationElement *value = nullptr;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationGridItemPattern *pPattern = static_cast<IUIAutomationGridItemPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentContainingGrid(&value);
            }
        }
        return reinterpret_cast<size_t>(value);
    }

    DLL_EXPORT int GridItemPatternCurrentRow(size_t pattern)
    {
        int value = -1;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationGridItemPattern *pPattern = static_cast<IUIAutomationGridItemPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentRow(&value);
            }
        }
        return value;
    }

    DLL_EXPORT int GridItemPatternCurrentColumn(size_t pattern)
    {
        int value = -1;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationGridItemPattern *pPattern = static_cast<IUIAutomationGridItemPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentColumn(&value);
            }
        }
        return value;
    }

    DLL_EXPORT int GridItemPatternCurrentRowSpan(size_t pattern)
    {
        int value = -1;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationGridItemPattern *pPattern = static_cast<IUIAutomationGridItemPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentRowSpan(&value);
            }
        }
        return value;
    }

    DLL_EXPORT int GridItemPatternCurrentColumnSpan(size_t pattern)
    {
        int value = -1;
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationGridItemPattern *pPattern = static_cast<IUIAutomationGridItemPattern*>(pUnknown);
            if (pPattern)
            {
                pPattern->get_CurrentColumnSpan(&value);
            }
        }
        return value;
    }

    DLL_EXPORT HRESULT TransformPatternMove(size_t pattern, int x, int y)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationTransformPattern *pPattern = static_cast<IUIAutomationTransformPattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->Move(x, y);
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT HRESULT TransformPatternResize(size_t pattern, int width, int height)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationTransformPattern *pPattern = static_cast<IUIAutomationTransformPattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->Resize(width, height);
            }
        }
        return S_FALSE;
    }

    DLL_EXPORT HRESULT TransformPatternRotate(size_t pattern, int degrees)
    {
        IUnknown *pUnknown = reinterpret_cast<IUnknown*>(pattern);
        if (pUnknown)
        {
            IUIAutomationTransformPattern *pPattern = static_cast<IUIAutomationTransformPattern*>(pUnknown);
            if (pPattern)
            {
                return pPattern->Rotate(degrees);
            }
        }
        return S_FALSE;
    }

    int GetImageEncoderClsid(const wchar_t* format, CLSID* pClsid)
    {
        UINT num = 0;          // number of image encoders
        UINT size = 0;         // size of the image encoder array in bytes

        ImageCodecInfo* pImageCodecInfo = NULL;

        GetImageEncodersSize(&num, &size);
        if(size == 0)
            return -1;  // Failure

        pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
        if(pImageCodecInfo == NULL)
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

        Bitmap* pBitmap = Bitmap::FromHBITMAP(hBitmap, NULL);

        DeleteObject(hBitmap);
        ReleaseDC(hWnd, hdc);

        return reinterpret_cast<size_t>(pBitmap);
    }

    DLL_EXPORT size_t BitmapFromFile(LPCTSTR path)
    {
        Bitmap* pBitmap = Bitmap::FromFile(path, NULL);
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
                pBitmap->Save(path, &imgClsId, NULL);
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

    DLL_EXPORT UINT BitmapGetPixel(size_t bitmap, UINT x, UINT y)
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

    DLL_EXPORT BOOL BitmapSetPixel(size_t bitmap, UINT x, UINT y, UINT argb)
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

    DLL_EXPORT BOOL BitmapGetPixelsHorizontally(size_t bitmap, UINT x, UINT y, UINT* array, UINT size)
    {
        if (bitmap && array)
        {
            Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
            UINT width = pBitmap->GetWidth();
            for (UINT n=0; n<size; ++n)
            {
                Color color;
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

    DLL_EXPORT BOOL BitmapGetPixelsVertically(size_t bitmap, UINT x, UINT y, UINT* array, UINT size)
    {
        if (bitmap && array)
        {
            Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
            UINT height = pBitmap->GetHeight();
            for (UINT n=0; n<size; ++n)
            {
                Color color;
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

    DLL_EXPORT BOOL BitmapSetPixelsHorizontally(size_t bitmap, UINT x, UINT y, UINT* array, UINT size)
    {
        if (bitmap && array)
        {
            Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
            UINT width = pBitmap->GetWidth();
            for (UINT n=0; n<size; ++n)
            {
                Color color(array[n]);
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

    DLL_EXPORT BOOL BitmapSetPixelsVertically(size_t bitmap, UINT x, UINT y, UINT* array, UINT size)
    {
        if (bitmap && array)
        {
            Bitmap* pBitmap = reinterpret_cast<Bitmap*>(bitmap);
            UINT height = pBitmap->GetHeight();
            for (UINT n=0; n<size; ++n)
            {
                Color color(array[n]);
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

#ifdef __cplusplus
}
#endif
