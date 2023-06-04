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
	INIFile config(GetModFolderPath() + "\\config.ini");
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

	Log("Field of view: ", fov.m128_f32[0]);
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(5s);
	
	Log("Activating AdjustTheFov...");
	std::string aob = "8d ? ? ? ? 0f 28 ? e8 ? ? ? ? 80 ? ? ? ? ? ? ? 0f 28 ? f3 ? 0f 10 ? ? ? ? ? ? 0f 57 ? f3 ? 0f 59";
	uintptr_t hookAddress = AobScan(aob);
	size_t offset = 1;

	if (hookAddress != 0)
	{
		ReadConfig();
		hookAddress -= offset;
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