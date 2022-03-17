#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating RemoveChromaticAberration...");
	std::vector<unsigned char> pattern = { 0x0f, 0x11, 0x43, 0x60, 0x48, 0x8d, 0x8b, 0x80, 0x00, 0x00, 0x00, 0x0f, 0x10, 0x87, 0xa0, 0x00, 0x00, 0x00, 0x0f, 0x11, 0x41, 0xf0, 0x48, 0x8d, 0x87, 0xb0, 0x00, 0x00, 0x00, 0x0f, 0x10, 0x08, 0x0f, 0x11, 0x09 };
	std::vector<unsigned char> originalBytes = { 0x0f, 0x11, 0x49, 0x20 };
	std::vector<unsigned char> newBytes = { 0x66, 0x0f, 0xef, 0xc9 };
	uintptr_t patchAddress = SigScan(pattern);
	if (patchAddress != 0)
	{
		patchAddress += 0x2f;
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