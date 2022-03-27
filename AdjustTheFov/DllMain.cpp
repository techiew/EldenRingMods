#include <Windows.h>
#include <xmmintrin.h>

#include "ModUtils.h"

using namespace ModUtils;
using namespace mINI;

extern "C" {
	void FovAdjust();
	__m128 fov = _mm_setr_ps(48.0f, 0.0f, 0.0f, 0.0f);
	uintptr_t returnAddress = 0;
	uintptr_t resolvedRelativeAddress = 0;
}

void ReadConfig()
{
	INIFile config(GetModuleFolderPath() + "\\config.ini");
	INIStructure ini;

	if (config.read(ini))
	{
		fov = _mm_setr_ps(std::stof(ini["fov"].get("value")), 0.0f, 0.0f, 0.0f);
	} 
	else
	{
		ini["fov"]["value"] = "48";
		config.write(ini, true);
	}

	Log("Field of view: %f", fov.m128_f32[0]);
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating AdjustTheFov...");
	std::vector<uint16_t> pattern = { 0x8d, MASKED, MASKED, MASKED, MASKED, 0x0f, 0x28, MASKED, 0xe8, MASKED, MASKED, MASKED, MASKED, 0x80, MASKED, MASKED, MASKED, MASKED, MASKED, MASKED, MASKED, 0x0f, 0x28, MASKED, 0xf3, MASKED, 0x0f, 0x10, MASKED, MASKED, MASKED, MASKED, MASKED, MASKED, 0x0f, 0x57, MASKED, 0xf3, MASKED, 0x0f, 0x59 };
	std::vector<uint8_t> originalBytes(9, 0x90);
	intptr_t unresolvedRelativeAddress = 0;
	uintptr_t hookAddress = SigScan(pattern);

	if (hookAddress != 0)
	{
		hookAddress -= 1;

		ReadConfig();

		DWORD oldProtection = 0;
		DWORD oldProtection2 = 0;

		VirtualProtect((void*)hookAddress, originalBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtection);
		memcpy(&originalBytes[0], (void*)hookAddress, originalBytes.size());
		memcpy(&unresolvedRelativeAddress, (void*)(hookAddress + 10), 4);
		VirtualProtect((void*)hookAddress, originalBytes.size(), oldProtection, &oldProtection);

		Log("Unresolved relative addr: 0x%llX", unresolvedRelativeAddress);

		VirtualProtect((void*)&FovAdjust, originalBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtection2);
		memcpy(&FovAdjust, &originalBytes[0], originalBytes.size());
		VirtualProtect((void*)&FovAdjust, originalBytes.size(), oldProtection2, &oldProtection2);

		returnAddress = hookAddress + 14;
		resolvedRelativeAddress = returnAddress + unresolvedRelativeAddress;

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