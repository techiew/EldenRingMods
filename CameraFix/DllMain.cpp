#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating camera fix...");
	std::vector<unsigned char> mask = { 3, 4, 5, 6 };
	std::vector<unsigned char> originalBytes = { 0x0f, 0x29, 0xa6, MASKED, MASKED, MASKED, MASKED, 0x41, 0x0f, 0x28, 0xcf, 0x48, 0x8b, 0xce };
	std::vector<unsigned char> newBytes(7, 0x90);
	uintptr_t patchAddress = SigScan(originalBytes, mask);
	if (patchAddress != 0)
	{
		Replace(patchAddress, originalBytes, newBytes, mask);
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