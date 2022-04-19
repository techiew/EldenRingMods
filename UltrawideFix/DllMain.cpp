#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating ultrawide fix...");
	std::vector<uint16_t> originalBytes = { 0x74, 0x50, MASKED, 0x8b, MASKED, MASKED, 0xdc, 0x03, 0x00, 0x00, MASKED, 0x85, MASKED, 0x74, MASKED, MASKED, 0x8b, MASKED, MASKED, 0x0f, 0xaf };
	std::vector<uint8_t> newBytes = { 0xeb };
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
		CreateThread(0, 0, &MainThread, 0, 0, NULL);
	}
	return 1;
}