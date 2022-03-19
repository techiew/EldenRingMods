#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating camera fix...");
	std::vector<uint16_t> originalBytes = { 0x0f, 0x29, 0xa6, MASKED, MASKED, MASKED, MASKED, 0x41, 0x0f, 0x28, 0xcf, 0x48, 0x8b, 0xce };
	std::vector<uint8_t> newBytes(7, 0x90);
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