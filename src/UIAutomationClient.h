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

	DLL_EXPORT BOOL InitInstance();

	DLL_EXPORT void ReleaseInstance();

	DLL_EXPORT int WcsCpy(wchar_t *dest, size_t size, const wchar_t* src);

	DLL_EXPORT size_t NewCharArray(UINT size);

	DLL_EXPORT void DeleteCharArray(size_t array);

	DLL_EXPORT size_t NewWCharArray(UINT size);

	DLL_EXPORT void DeleteWCharArray(size_t array);

	DLL_EXPORT DWORD GetConsoleWindowTitle(wchar_t *dest, UINT size);

	DLL_EXPORT UINT SendUnicodeChar(WCHAR *pwChar);

	DLL_EXPORT int GetParentProcessId(int nProcessId);

	DLL_EXPORT size_t GetProcessCommandLine(UINT nProcessId);

	DLL_EXPORT void FreeBSTR(size_t bstr);

	DLL_EXPORT size_t GetAutomationObject();

	DLL_EXPORT size_t GetRawTreeWalker();

	DLL_EXPORT ULONG ElementAddRef(size_t element);

	DLL_EXPORT BOOL CompareElements(size_t element1, size_t element2);

	DLL_EXPORT ULONG ReleaseElement(size_t element);

	DLL_EXPORT ULONG ReleaseElementArray(size_t elementArray);

	DLL_EXPORT size_t GetRootElement();

	DLL_EXPORT size_t GetFocusedElement();

	DLL_EXPORT size_t ElementFromPoint(int x, int y);

	DLL_EXPORT size_t ElementFromHandle(size_t handle);

	DLL_EXPORT size_t GetElementName(size_t element);

	DLL_EXPORT size_t GetElementControlTypeName(size_t element);

	DLL_EXPORT size_t GetElementClassName(size_t element);

	DLL_EXPORT size_t GetElementAutomationId(size_t element);

	DLL_EXPORT int GetElementProcessId(size_t element);

	DLL_EXPORT int GetElementControlType(size_t element);

	DLL_EXPORT size_t GetElementLocalizedControlType(size_t element);

	DLL_EXPORT HRESULT GetElementBoundingRectangle(size_t element, RECT* pRect);

	DLL_EXPORT BOOL GetElementIsEnabled(size_t element);

	DLL_EXPORT BOOL GetElementHasKeyboardFocus(size_t element);

	DLL_EXPORT BOOL GetElementIsKeyboardFocusable(size_t element);

	DLL_EXPORT BOOL GetElementIsOffscreen(size_t element);

	DLL_EXPORT size_t GetElementHandle(size_t element);

	DLL_EXPORT HRESULT SetElementFocus(size_t element);

	DLL_EXPORT size_t GetParentElement(size_t element);

	DLL_EXPORT size_t GetNextSiblingElement(size_t element);

	DLL_EXPORT size_t GetPreviousSiblingElement(size_t element);

	DLL_EXPORT size_t GetFirstChildElement(size_t element);

	DLL_EXPORT size_t GetLastChildElement(size_t element);

	DLL_EXPORT int ElementArrayGetLength(size_t elementArray);

	DLL_EXPORT size_t ElementArrayGetElement(size_t elementArray, int index);

	DLL_EXPORT size_t GetElementPattern(size_t element, int patternId);

	DLL_EXPORT ULONG ReleasePattern(size_t pattern);

	DLL_EXPORT HRESULT InvokePatternInvoke(size_t pattern);

	DLL_EXPORT HRESULT TogglePatternToggle(size_t pattern);

	DLL_EXPORT int TogglePatternCurrentToggleState(size_t pattern);

	DLL_EXPORT HRESULT ExpandCollapsePatternExpand(size_t pattern);

	DLL_EXPORT HRESULT ExpandCollapsePatternCollapse(size_t pattern);

	DLL_EXPORT int ExpandCollapsePatternCurrentExpandCollapseState(size_t pattern);

	DLL_EXPORT size_t ValuePatternCurrentValue(size_t pattern);

	DLL_EXPORT HRESULT ValuePatternSetValue(size_t pattern, WCHAR* pwValue);

	DLL_EXPORT BOOL ValuePatternCurrentIsReadOnly(size_t pattern);

	DLL_EXPORT HRESULT ScrollItemPatternScrollIntoView(size_t pattern);

	DLL_EXPORT BOOL ScrollPatternCurrentHorizontallyScrollable(size_t pattern);

	DLL_EXPORT int ScrollPatternCurrentHorizontalViewSize(size_t pattern);

	DLL_EXPORT int ScrollPatternCurrentHorizontalScrollPercent(size_t pattern);

	DLL_EXPORT BOOL ScrollPatternCurrentVerticallyScrollable(size_t pattern);

	DLL_EXPORT int ScrollPatternCurrentVerticalViewSize(size_t pattern);

	DLL_EXPORT int ScrollPatternCurrentVerticalScrollPercent(size_t pattern);

	DLL_EXPORT HRESULT ScrollPatternSetScrollPercent(size_t pattern, int hPercent, int vPercent);

	DLL_EXPORT size_t SelectionPatternGetCurrentSelection(size_t pattern);

	DLL_EXPORT HRESULT SelectionItemPatternSelect(size_t pattern);

	DLL_EXPORT HRESULT SelectionItemPatternAddToSelection(size_t pattern);

	DLL_EXPORT HRESULT SelectionItemPatternRemoveFromSelection(size_t pattern);

	DLL_EXPORT BOOL SelectionItemPatternCurrentIsSelected(size_t pattern);

	DLL_EXPORT int RangeValuePatternCurrentValue(size_t pattern);

	DLL_EXPORT HRESULT RangeValuePatternSetValue(size_t pattern, int value);

	DLL_EXPORT int RangeValuePatternCurrentMaximum(size_t pattern);

	DLL_EXPORT int RangeValuePatternCurrentMinimum(size_t pattern);

	DLL_EXPORT int WindowPatternCurrentWindowVisualState(size_t pattern);

	DLL_EXPORT HRESULT WindowPatternSetWindowVisualState(size_t pattern, int state);

	DLL_EXPORT BOOL WindowPatternCurrentCanMaximize(size_t pattern);

	DLL_EXPORT BOOL WindowPatternCurrentCanMinimize(size_t pattern);

	DLL_EXPORT BOOL WindowPatternCurrentIsModal(size_t pattern);

	DLL_EXPORT BOOL WindowPatternCurrentIsTopmost(size_t pattern);

	DLL_EXPORT HRESULT WindowPatternClose(size_t pattern);

	DLL_EXPORT HRESULT LegacyIAccessiblePatternSelect(size_t pattern, long flag);

	DLL_EXPORT HRESULT LegacyIAccessiblePatternDoDefaultAction(size_t pattern);

	DLL_EXPORT HRESULT LegacyIAccessiblePatternSetValue(size_t pattern, WCHAR* pwValue);

	DLL_EXPORT int LegacyIAccessiblePatternCurrentChildId(size_t pattern);

	DLL_EXPORT size_t LegacyIAccessiblePatternCurrentName(size_t pattern);

	DLL_EXPORT size_t LegacyIAccessiblePatternCurrentValue(size_t pattern);

	DLL_EXPORT size_t LegacyIAccessiblePatternCurrentDescription(size_t pattern);

	DLL_EXPORT DWORD LegacyIAccessiblePatternCurrentRole(size_t pattern);

	DLL_EXPORT DWORD LegacyIAccessiblePatternCurrentState(size_t pattern);

	DLL_EXPORT size_t LegacyIAccessiblePatternCurrentHelp(size_t pattern);

	DLL_EXPORT size_t LegacyIAccessiblePatternCurrentKeyboardShortcut(size_t pattern);

	DLL_EXPORT size_t LegacyIAccessiblePatternGetCurrentSelection(size_t pattern);

	DLL_EXPORT size_t LegacyIAccessiblePatternCurrentDefaultAction(size_t pattern);

	DLL_EXPORT size_t GridPatternGetItem(size_t pattern, int row, int column);

	DLL_EXPORT int GridPatternCurrentRowCount(size_t pattern);

	DLL_EXPORT int GridPatternCurrentColumnCount(size_t pattern);

	DLL_EXPORT size_t TablePatternCurrentRowHeaders(size_t pattern);

	DLL_EXPORT size_t TablePatternCurrentColumnHeaders(size_t pattern);

	DLL_EXPORT int TablePatternCurrentRowOrColumnMajor(size_t pattern);

	DLL_EXPORT size_t TableItemPatternCurrentRowHeaderItems(size_t pattern);

	DLL_EXPORT size_t TableItemPatternCurrentColumnHeaderItems(size_t pattern);

	DLL_EXPORT size_t GridItemPatternCurrentContainingGrid(size_t pattern);

	DLL_EXPORT int GridItemPatternCurrentRow(size_t pattern);

	DLL_EXPORT int GridItemPatternCurrentColumn(size_t pattern);

	DLL_EXPORT int GridItemPatternCurrentRowSpan(size_t pattern);

	DLL_EXPORT int GridItemPatternCurrentColumnSpan(size_t pattern);

	DLL_EXPORT HRESULT TransformPatternMove(size_t pattern, int x, int y);

	DLL_EXPORT HRESULT TransformPatternResize(size_t pattern, int width, int height);

	DLL_EXPORT HRESULT TransformPatternRotate(size_t pattern, int degrees);

	DLL_EXPORT size_t BitmapCreate(int width, int height);

	DLL_EXPORT size_t BitmapFromWindow(size_t wnd, int left, int top, int right, int bottom);

	DLL_EXPORT size_t BitmapFromFile(LPCTSTR path);

	DLL_EXPORT BOOL BitmapToFile(size_t bitmap, LPCTSTR path, LPCTSTR gdiplusFormat);

	DLL_EXPORT void BitmapRelease(size_t bitmap);

	DLL_EXPORT UINT BitmapGetWidthAndHeight(size_t bitmap);

	DLL_EXPORT UINT BitmapGetPixel(size_t bitmap, UINT x, UINT y);

	DLL_EXPORT BOOL BitmapSetPixel(size_t bitmap, UINT x, UINT y, UINT argb);

	DLL_EXPORT BOOL BitmapGetPixelsHorizontally(size_t bitmap, UINT x, UINT y, UINT* array, UINT size);

	DLL_EXPORT BOOL BitmapGetPixelsVertically(size_t bitmap, UINT x, UINT y, UINT* array, UINT size);

	DLL_EXPORT BOOL BitmapSetPixelsHorizontally(size_t bitmap, UINT x, UINT y, UINT* array, UINT size);

	DLL_EXPORT BOOL BitmapSetPixelsVertically(size_t bitmap, UINT x, UINT y, UINT* array, UINT size);


#ifdef __cplusplus
}
#endif

