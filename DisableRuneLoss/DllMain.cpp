#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating DisableRuneLoss...");
	std::vector<uint16_t> pattern = { 0x41, MASKED, 0x01, 0x48, MASKED, MASKED, 0xe8, MASKED, MASKED, MASKED, MASKED, 0x48, MASKED, MASKED, MASKED, MASKED, 0x32, 0xc0 };
	std::vector<uint16_t> originalBytes = { 0xe8, MASKED, MASKED, MASKED, MASKED };
	std::vector<uint8_t> newBytes = { 0x90, 0x90, 0x90, 0x90, 0x90 };
	uintptr_t patchAddress = SigScan(pattern);
	if (patchAddress != 0)
	{
		patchAddress += 6;
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