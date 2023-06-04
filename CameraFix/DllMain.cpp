#include <thread>
#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;
using namespace mINI;

bool disableCameraAutoRotate = true;
bool disableCameraReset = true;

void ReadConfig()
{
	INIFile config(GetModFolderPath() + "\\config.ini");
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

	Log("Disable camera auto rotate: ", disableCameraAutoRotate);
	Log("Disable camera reset: ", disableCameraReset);
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(5s);
	
	Log("Activating camera fixes...");

	ReadConfig();

	if (disableCameraAutoRotate)
	{
		std::string aob = "0f 29 ? ? ? ? ? ? 0f 28 ? ? 8b ? e8 ? ? ? ? ? 0f b6 ? ? ? ? 0f 28 ? ? 8b ? e8 ? ? ? ? ? 8b ? ? 0f 28 ? ? 8b ? e8 ? ? ? ? ? 8d ? ? ? 8b ? ? 8d";
		std::string newBytes = "90 90 90 90 90 90 90";
		uintptr_t patchAddress = AobScan(aob);
		if (patchAddress != 0)
		{
			ReplaceExpectedBytesAtAddress(patchAddress, aob, newBytes);
		}
	}

	if (disableCameraReset)
	{
		std::string aob = "80 ? ? ? ? ? 00 74 ? ? 8b ? e8 ? ? ? ? eb ? 0f 28 ? ? ? ? ? ? 8d";
		std::string expectedBytes = "74";
		std::string newBytes = "eb";
		uintptr_t patchAddress = AobScan(aob);
		size_t offset = 7;
		if (patchAddress != 0)
		{
			patchAddress += offset;
			ReplaceExpectedBytesAtAddress(patchAddress, expectedBytes, newBytes);
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