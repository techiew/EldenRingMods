#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating ultrawide fix...");
	std::vector<uint16_t> originalBytes = { 0x8b, MASKED, 0x85, MASKED, 0x74, MASKED, MASKED, 0x8b, MASKED, 0x04, MASKED, 0x85, MASKED, 0x74, MASKED, 0x41, 0x8b, MASKED, MASKED, 0x0f, 0xaf };
	std::vector<uint8_t> newBytes = { 0x8b, 0x01, 0x85, 0xc0, 0xeb, 0x42, 0x44, 0x8b, 0x59, 0x04 };
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