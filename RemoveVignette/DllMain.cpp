#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating RemoveVignette...");
	std::vector<unsigned char> mask = { 4, 5, 6, 7 };
	std::vector<unsigned char> originalBytes = { 0xf3, 0x0f, 0x59, 0x05, MASKED, MASKED, MASKED, MASKED, 0xf3, 0x44, 0x0f, 0x5c, 0xc8, 0xf3, 0x45, 0x0f, 0x5e, 0xcb };
	std::vector<unsigned char> newBytes = { 0xf3, 0x0f, 0x59, 0x05, 0x24, 0xb5, 0x6e, 0x01, 0xf3, 0x44, 0x0f, 0x5c, 0xc8, 0xf3, 0x45, 0x0f, 0x5e, 0xcb };
	uintptr_t patchAddress = SigScan(originalBytes, mask);
	if (patchAddress != 0)
	{
		Replace(patchAddress, originalBytes, newBytes, mask);
	}
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