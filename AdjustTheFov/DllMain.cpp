#include <thread>
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
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(5s);
	
	Log("Activating AdjustTheFov...");
	std::vector<uint16_t> pattern = { 0x8d, MASKED, MASKED, MASKED, MASKED, 0x0f, 0x28, MASKED, 0xe8, MASKED, MASKED, MASKED, MASKED, 0x80, MASKED, MASKED, MASKED, MASKED, MASKED, MASKED, MASKED, 0x0f, 0x28, MASKED, 0xf3, MASKED, 0x0f, 0x10, MASKED, MASKED, MASKED, MASKED, MASKED, MASKED, 0x0f, 0x57, MASKED, 0xf3, MASKED, 0x0f, 0x59 };
	uintptr_t hookAddress = SigScan(pattern);

	if (hookAddress != 0)
	{
		ReadConfig();
		hookAddress -= 1;
		size_t size = 9;
		MemCopy((uintptr_t)&FovAdjust, hookAddress, size);
		returnAddress = hookAddress + 14;
		resolvedRelativeAddress = RelativeToAbsoluteAddress(hookAddress + 10);

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