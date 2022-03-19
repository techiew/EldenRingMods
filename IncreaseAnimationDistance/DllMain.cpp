#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating IncreaseAnimationDistance...");
	std::vector<uint16_t> originalBytes = {  };
	std::vector<uint8_t> newBytes = {  };
	uintptr_t patchAddress = SigScan(originalBytes);
	if (patchAddress != 0)
	{
		Replace(patchAddress, originalBytes, newBytes);
	}
	CloseLog();
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(module);
		//CreateThread(0, 0, &MainThread, 0, 0, NULL);
	}
	return 1;
}