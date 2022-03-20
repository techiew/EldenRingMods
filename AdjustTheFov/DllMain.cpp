#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

extern "C" {
	void FovAdjust();
	float fov = 100.0f;
	uintptr_t returnAddress = 0;
	uintptr_t resolvedRelativeAddress = 0;
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating AdjustTheFov...");
	std::vector<uint16_t> pattern = { 0x48, 0x8d, 0x4c, 0x24, 0x20, 0x44, 0x0f, 0x28, 0xc8, 0xe8, MASKED, MASKED, MASKED, MASKED, 0x80, 0xbb, 0x88, 0x04, 0x00, 0x00, 0x00, 0x44, 0x0f, 0x28, 0xe0 };
	std::vector<uint8_t> originalBytes(9, 0x90);
	intptr_t unresolvedRelativeAddress = 0;
	uintptr_t hookAddress = SigScan(pattern);
	if (hookAddress != 0)
	{
		DWORD oldProtection = 0;
		DWORD oldProtection2 = 0;

		VirtualProtect((void*)hookAddress, originalBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtection);
		memcpy(&originalBytes[0], (void*)hookAddress, originalBytes.size());
		memcpy(&unresolvedRelativeAddress, (void*)(hookAddress + 10), sizeof(intptr_t));
		VirtualProtect((void*)hookAddress, originalBytes.size(), oldProtection, &oldProtection);

		Log("Unresolved relative addr: %x", unresolvedRelativeAddress);

		VirtualProtect((void*)&FovAdjust, originalBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtection2);
		memcpy(&FovAdjust, &originalBytes[0], originalBytes.size());
		VirtualProtect((void*)&FovAdjust, originalBytes.size(), oldProtection2, &oldProtection2);

		returnAddress = hookAddress + 14;
		resolvedRelativeAddress = hookAddress - unresolvedRelativeAddress;

		Hook(hookAddress, (uintptr_t)&FovAdjust);
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