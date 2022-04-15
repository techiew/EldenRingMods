#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

HWND eldenRingWindow = NULL;
DWORD WINAPI MainThread(LPVOID lpParam)
{
	if (GetWindowHandle())
	{
		eldenRingWindow = muWindow;
	}

	Log("Activating SkipTheIntro...");
	std::vector<uint16_t> pattern = { 0x48, 0x8B, 0x90, MASKED, MASKED, MASKED, MASKED, 0x48, 0x85, 0xD2, 0x74, 0x07, 0xC6 };
	std::vector<uint16_t> originalBytes = { 0x48, 0x8B, 0x90, 0x80, 0x00, 0x00, 0x00 };
	std::vector<uint8_t> newBytes = { 0xE9, 0x1C, 0x00, 0x00, 0x00 };
	uintptr_t patchAddress = SigScan(pattern);

	if (patchAddress != 0)
	{
		if (!Replace(patchAddress, originalBytes, newBytes))
		{
			Log("PATCH FAILED");
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
