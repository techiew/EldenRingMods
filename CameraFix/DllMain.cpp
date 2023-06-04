#include <thread>
#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;
using namespace mINI;

static bool disableCameraAutoRotate = true;
static bool disableCameraReset = true;

void ReadConfig()
{
	INIFile config(GetModuleFolderPath() + "\\config.ini");
	INIStructure ini;

	if (config.read(ini))
	{
		disableCameraAutoRotate = stoi(ini["fix_the_camera"]["disable_camera_auto_rotate"]) > 0;
		disableCameraReset = stoi(ini["fix_the_camera"]["disable_camera_reset"]) > 0;
	}
	else
	{
		ini["fix_the_camera"]["disable_camera_auto_rotate"] = "1";
		ini["fix_the_camera"]["disable_camera_reset"] = "1";
		config.write(ini, true);
	}

	Log("Disable camera auto rotate: %i", disableCameraAutoRotate);
	Log("Disable camera reset: %i", disableCameraReset);
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(5s);
	
	Log("Activating camera fixes...");

	ReadConfig();

	if (disableCameraAutoRotate)
	{
		std::vector<uint16_t> originalBytes = { 0x0f, 0x29, MASKED, MASKED, MASKED, MASKED, MASKED, MASKED, 0x0f, 0x28, MASKED, MASKED, 0x8b, MASKED, 0xe8, MASKED, MASKED, MASKED, MASKED, MASKED, 0x0f, 0xb6, MASKED, MASKED, MASKED, MASKED, 0x0f, 0x28, MASKED, MASKED, 0x8b, MASKED, 0xe8, MASKED, MASKED, MASKED, MASKED, MASKED, 0x8b, MASKED, MASKED, 0x0f, 0x28, MASKED, MASKED, 0x8b, MASKED, 0xe8, MASKED, MASKED, MASKED, MASKED, MASKED, 0x8d, MASKED, MASKED, MASKED, 0x8b, MASKED, MASKED, 0x8d };
		std::vector<uint8_t> newBytes(7, 0x90);
		uintptr_t patchAddress = SigScan(originalBytes);
		if (patchAddress != 0)
		{
			Replace(patchAddress, originalBytes, newBytes);
		}
	}

	if (disableCameraReset)
	{
		std::vector<uint16_t> pattern = { 0x80, MASKED, MASKED, MASKED, MASKED, MASKED, 0x00, 0x74, MASKED, MASKED, 0x8b, MASKED, 0xe8, MASKED, MASKED, MASKED, MASKED, 0xeb, MASKED, 0x0f, 0x28, MASKED, MASKED, MASKED, MASKED, MASKED, MASKED, 0x8d };
		std::vector<uint16_t> originalBytes = { 0x74 };
		std::vector<uint8_t> newBytes = { 0xeb };
		uintptr_t patchAddress = SigScan(pattern);
		if (patchAddress != 0)
		{
			patchAddress += 7;
			Replace(patchAddress, originalBytes, newBytes);
		}
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