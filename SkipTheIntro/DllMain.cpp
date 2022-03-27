#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating SkipTheIntro...");
	std::vector<uint16_t> pattern = { 0xc6, MASKED, MASKED, MASKED, MASKED, MASKED, 0x01, MASKED, 0x03, 0x00, 0x00, 0x00, MASKED, 0x8b, MASKED, 0xe8, MASKED, MASKED, MASKED, MASKED, 0xe9, MASKED, MASKED, MASKED, MASKED, MASKED, 0x8d };
	std::vector<uint16_t> originalBytes = { 0x80, MASKED, MASKED, MASKED, MASKED, MASKED, 0x00 };
	std::vector<uint8_t> newBytes = { 0x80, 0xbf, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90, 0x48 };
	uintptr_t patchAddress = SigScan(pattern);
	if (patchAddress != 0)
	{
		patchAddress -= 0x43;
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