#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating UnlockTheFps...");
	std::vector<unsigned char> pattern = { 0xc7, 0x43, 0x20, 0x89, 0x88, 0x88, 0x3c, 0xeb, 0x43, 0x89, 0x73, 0x18, 0xeb, 0xca, 0x89, 0x73, 0x18 };
	std::vector<unsigned char> originalBytes = { 0x89, 0x88, 0x88, 0x3c };
	std::vector<unsigned char> newBytes = { 0x3a, 0x83, 0x34, 0x05 };
	uintptr_t patchAddress = SigScan(pattern);
	if (patchAddress == 0)
	{
		return 1;
	}
	patchAddress += 0x3;
	if (!Replace(patchAddress, originalBytes, newBytes))
	{
		return 1;
	}
	
	Log("Removing 60 FPS fullscreen limit...");
	originalBytes = { 0xc7, 0x45, 0xef, 0x3c, 0x00, 0x00, 0x00 };
	newBytes = { 0xc7, 0x45, 0xef, 0x00, 0x00, 0x00, 0x00 };
	patchAddress = SigScan(originalBytes);
	if (patchAddress != 0)
	{
		Replace(patchAddress, originalBytes, newBytes);
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