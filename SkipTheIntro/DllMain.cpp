#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;
using namespace mINI;

HWND eldenRingWindow = NULL;
HWND antiFlashbangWindow = NULL;

bool skipIntroLogos = true;
bool hideInitialWhiteScreen = true;
unsigned int hideWhiteScreenDurationMs = 10000;

void ShowAntiFlashbangWindow()
{
	HINSTANCE hInstance = GetModuleHandleA(GetCurrentProcessName().c_str());
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
	INIFile config(GetModFolderPath() + "\\config.ini");
	INIStructure ini;

	if (config.read(ini))
	{
		skipIntroLogos = stoi(ini["skip_the_intro"]["skip_intro_logos"]) > 0;
		hideInitialWhiteScreen = stoi(ini["skip_the_intro"]["hide_initial_white_screen"]) > 0;
		hideWhiteScreenDurationMs = stoi(ini["skip_the_intro"]["hide_initial_white_screen_duration"]);
	}
	else
	{
		ini["skip_the_intro"]["skip_intro_logos"] = "1";
		ini["skip_the_intro"]["hide_initial_white_screen"] = "1";
		ini["skip_the_intro"]["hide_initial_white_screen_duration"] = std::to_string(hideWhiteScreenDurationMs);
		config.write(ini, true);
	}

	Log("Skip intro logos: ", skipIntroLogos);
	Log("Hide initial white screen: ", hideInitialWhiteScreen);
	Log("Hide initial white screen duration: ", hideWhiteScreenDurationMs);
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
		std::string aob = "c6 ? ? ? ? ? 01 ? 03 00 00 00 ? 8b ? e8 ? ? ? ? e9 ? ? ? ? ? 8d";
		std::string expectedBytes = "74";
		std::string newBytes = "90 90";
		uintptr_t patchAddress = AobScan(aob);
		size_t offset = 60;
		if (patchAddress != 0)
		{
			patchAddress -= offset;
			ReplaceExpectedBytesAtAddress(patchAddress, expectedBytes, newBytes);
		}
	}

	if (hideInitialWhiteScreen)
	{
		Timer closeWindowTimer(hideWhiteScreenDurationMs);
		while (true)
		{
			if (closeWindowTimer.Check())
			{
				Log("Closing window");
				SetWindowPos(antiFlashbangWindow, HWND_BOTTOM, 0, 0, 0, 0, NULL);
				ShowWindow(antiFlashbangWindow, SW_HIDE);
				PostMessage(antiFlashbangWindow, WM_CLOSE, NULL, NULL);
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