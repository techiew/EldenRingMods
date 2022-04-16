#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;
using namespace mINI;

HWND eldenRingWindow = NULL;
HWND antiFlashbangWindow = NULL;

bool skipIntroLogos = true;
bool hideInitialWhiteScreen = true;
unsigned int hideWhiteScreenDuration = 10000;

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
		WS_EX_TOPMOST,
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

	SetWindowPos(antiFlashbangWindow, HWND_TOPMOST, x, y, width, height, NULL);
	ShowWindow(antiFlashbangWindow, SW_SHOWNORMAL);
	UpdateWindow(antiFlashbangWindow);
}

void ReadConfigFile()
{
	INIFile config(GetModuleFolderPath() + "\\config.ini");
	INIStructure ini;

	if (config.read(ini))
	{
		skipIntroLogos = stoi(ini["skip_the_intro"]["skip_intro_logos"]) > 0;
		hideInitialWhiteScreen = stoi(ini["skip_the_intro"]["hide_initial_white_screen"]) > 0;
		hideWhiteScreenDuration = stoi(ini["skip_the_intro"]["hide_initial_white_screen_duration"]);
	}
	else
	{
		ini["skip_the_intro"]["skip_intro_logos"] = "1";
		ini["skip_the_intro"]["hide_initial_white_screen"] = "1";
		ini["skip_the_intro"]["hide_initial_white_screen_duration"] = std::to_string(hideWhiteScreenDuration);
		config.write(ini, true);
	}

	Log("Skip intro logos: %i", skipIntroLogos);
	Log("Hide initial white screen: %i", hideInitialWhiteScreen);
	Log("Hide initial white screen duration: %i", hideWhiteScreenDuration);
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	ReadConfigFile();

	if (hideInitialWhiteScreen)
	{
		if (GetWindowHandle())
		{
			eldenRingWindow = muWindow;
		}
		ShowAntiFlashbangWindow();
	}

	if (skipIntroLogos)
	{
		Log("Activating SkipTheIntro...");
		std::vector<uint16_t> pattern = { 0xc6, MASKED, MASKED, MASKED, MASKED, MASKED, 0x01, MASKED, 0x03, 0x00, 0x00, 0x00, MASKED, 0x8b, MASKED, 0xe8, MASKED, MASKED, MASKED, MASKED, 0xe9, MASKED, MASKED, MASKED, MASKED, MASKED, 0x8d };
		std::vector<uint16_t> originalBytes = { 0x74 };
		std::vector<uint8_t> newBytes = { 0x90, 0x90 };
		uintptr_t patchAddress = SigScan(pattern);

		if (patchAddress != 0)
		{
			patchAddress -= 60;
			Replace(patchAddress, originalBytes, newBytes);
		}
	}

	if (hideInitialWhiteScreen)
	{
		Timer closeWindowTimer(hideWhiteScreenDuration);
		while (true)
		{
			if (closeWindowTimer.Check())
			{
				Log("Closing window");
				SetWindowPos(antiFlashbangWindow, HWND_BOTTOM, 0, 0, 0, 0, NULL);
				ShowWindow(antiFlashbangWindow, SW_HIDE);
				PostMessage(antiFlashbangWindow, WM_CLOSE, NULL, NULL);
			}

			MSG msg;
			if (GetMessage(&msg, NULL, 0, 0) > 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (IsWindow(antiFlashbangWindow) == false)
			{
				Log("Exiting");
				break;
			}

			Sleep(10);
		}
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