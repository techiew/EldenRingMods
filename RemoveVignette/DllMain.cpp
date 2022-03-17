#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating RemoveVignette...");
	std::vector<unsigned char> mask = { 3, 4, 8, 9, 10, 11, 12, 14, 15, 16, 17, 21, 22, 26, 27, 30, 31, 32, 33, 34, 35 };
	std::vector<unsigned char> pattern = { 0xf3, 0x0f, 0x10, MASKED, MASKED, 0xf3, 0x0f, 0x59, MASKED, MASKED, MASKED, MASKED, MASKED, 0xe8, MASKED, MASKED, MASKED, MASKED, 0xf3, 0x41, 0x0f, MASKED, MASKED, 0xf3, 0x45, 0x0f, MASKED, MASKED, 0x4c, 0x8d, MASKED, MASKED, MASKED, MASKED, MASKED, MASKED, 0x48 };
	std::vector<unsigned char> originalBytes = { 0xf3, 0x45, 0x0f, 0x59, 0xc2 };
	std::vector<unsigned char> newBytes = { 0xf3, 0x0f, 0x5c, 0xc0, 0x90 };
	uintptr_t patchAddress = SigScan(pattern, mask);
	if (patchAddress != 0)
	{
		patchAddress += 0x17;
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