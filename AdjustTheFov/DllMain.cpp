#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

extern "C" {
	void FovAdjust();
	float fov = 100.0f;
	uintptr_t returnAddr = 0;
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating AdjustTheFov...");
	std::vector<uint16_t> pattern = { 0x80, 0xBB, MASKED, MASKED, MASKED, MASKED, 0x00, 0x44, MASKED, MASKED, 0xE0, 0xF3, 0x44, MASKED, MASKED, MASKED, MASKED, MASKED, MASKED, MASKED, 0x45 };
	uintptr_t hookAddress = SigScan(pattern);
	if (hookAddress != 0)
	{
		size_t clearance = 20;
		DWORD oldProtection;
		VirtualProtect(&FovAdjust, clearance + 4, PAGE_EXECUTE_READWRITE, &oldProtection);
		memcpy((void*)((uintptr_t)&FovAdjust + 7), (void*)hookAddress, clearance);
		VirtualProtect(&FovAdjust, clearance + 4, oldProtection, &oldProtection);
		returnAddr = hookAddress + clearance;
		Hook(hookAddress, (uintptr_t)&FovAdjust, clearance);
	}
	CloseLog();
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(module);
		//CreateThread(0, 0, &MainThread, 0, 0, NULL);
	}
	return 1;
}