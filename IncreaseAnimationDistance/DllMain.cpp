#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating IncreaseAnimationDistance...");
	std::vector<uint16_t> pattern = { 0xc7, MASKED, MASKED, MASKED, 0x01, 0x00, 0x00, 0x00, 0xf3, MASKED, 0x0f, 0x10, MASKED, MASKED, MASKED, 0xf3, MASKED, 0x0f, 0x10, MASKED, MASKED, MASKED, 0xf3, 0x0f, 0x59, MASKED, MASKED, MASKED, MASKED, MASKED, MASKED, 0x0f, 0x28, MASKED, 0xf3, MASKED, 0x0f, 0x5c, MASKED, MASKED, 0x58 };
	std::vector<uint16_t> originalBytes = { 0xf3, MASKED, 0x0f, 0x5e, MASKED, MASKED, MASKED };
	std::vector<uint8_t> newBytes = { 0x0f, 0x57, 0xc9, 0x90, 0x90, 0x90, 0x90 };
	uintptr_t patchAddress = SigScan(pattern);
	if (patchAddress != 0)
	{
		patchAddress += 0x48;
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