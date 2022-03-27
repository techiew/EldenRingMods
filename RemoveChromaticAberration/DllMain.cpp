#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating RemoveChromaticAberration...");
	std::vector<uint16_t> pattern = { 0x0f, 0x11, MASKED, 0x60, MASKED, 0x8d, MASKED, 0x80, 0x00, 0x00, 0x00, 0x0f, 0x10, MASKED, 0xa0, 0x00, 0x00, 0x00, 0x0f, 0x11, MASKED, 0xf0, MASKED, 0x8d, MASKED, 0xb0, 0x00, 0x00, 0x00, 0x0f, 0x10, MASKED, 0x0f, 0x11, MASKED, 0x0f, 0x10, MASKED, 0x10 };
	std::vector<uint16_t> originalBytes = { 0x0f, 0x11, MASKED, MASKED };
	std::vector<uint8_t> newBytes = { 0x66, 0x0f, 0xef, 0xc9 };
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