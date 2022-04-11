#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

HWND eldenRingWindow = NULL;
HWND antiFlashbangWindow = NULL;

extern "C" {
	void OnIntroSkipped();
	uintptr_t returnAddress = 0;
	unsigned char introSkipped = 0;
}

void ShowAntiFlashbangWindow()
{
	HINSTANCE hInstance = GetModuleHandleA(GetModuleName().c_str());
	const char className[] = "AntiFlashbang";
	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = DefWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNTEXT + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = className;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&wc);
	antiFlashbangWindow = CreateWindowEx(
		NULL,
		className,
		"Anti-flashbang window",
		WS_POPUP,
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		0,
		0,
		NULL,
		NULL,
		hInstance, 
		NULL);

	RECT eldenRingRect;
	GetWindowRect(eldenRingWindow, &eldenRingRect);
	int width = eldenRingRect.right - eldenRingRect.left;
	int height = eldenRingRect.bottom - eldenRingRect.top;
	int x = eldenRingRect.left;
	int y = eldenRingRect.top;

	SetParent(antiFlashbangWindow, eldenRingWindow);
	SetWindowPos(antiFlashbangWindow, NULL, x, y, width, height, NULL);
	ShowWindow(antiFlashbangWindow, SW_MAXIMIZE);
	UpdateWindow(antiFlashbangWindow);
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	if (GetWindowHandle())
	{
		eldenRingWindow = muWindow;
	}
    ShowAntiFlashbangWindow();

	Log("Activating SkipTheIntro...");
	std::vector<uint16_t> pattern = { 0xc6, MASKED, MASKED, MASKED, MASKED, MASKED, 0x01, MASKED, 0x03, 0x00, 0x00, 0x00, MASKED, 0x8b, MASKED, 0xe8, MASKED, MASKED, MASKED, MASKED, 0xe9, MASKED, MASKED, MASKED, MASKED, MASKED, 0x8d };
	std::vector<uint16_t> originalBytes = { 0x74 };
	std::vector<uint8_t> newBytes = { 0x90, 0x90 };
	uintptr_t patchAddress = SigScan(pattern);

	if (patchAddress != 0)
	{
		patchAddress -= 60;

		if (Replace(patchAddress, originalBytes, newBytes))
		{
			patchAddress -= 31;
			size_t size = 19;
			MemCopy((uintptr_t)&OnIntroSkipped, patchAddress, size);
			returnAddress = patchAddress + size;
			Hook(patchAddress, (uintptr_t)&OnIntroSkipped, size - 14);
		}
	}

    Timer forceDestroyTimer(60000);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
		if (introSkipped || forceDestroyTimer.Check())
		{
			Log("Destroying window");
			DestroyWindow(antiFlashbangWindow);
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
		Sleep(10);
    }

	CloseLog();
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(module);
		CreateThread(0, 0, &MainThread, 0, 0, NULL);
	}
	return 1;
}