#include <Windows.h>
#include <algorithm>
#include <thread>

#include "ModUtils.h"

using namespace ModUtils;
using namespace mINI;

static float fpsLimit = 300;

void ReadConfig()
{
	INIFile config(GetModFolderPath() + "\\config.ini");
	INIStructure ini;

	if (config.read(ini))
	{
		fpsLimit = std::stof(ini["unlockthefps"].get("limit"));
	}
	else
	{
		ini["unlockthefps"]["limit"] = "300";
		config.write(ini, true);
	}

	Log("FPS limit: ", fpsLimit);
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(5s);
	
	Log("Activating UnlockTheFps...");
	{
		ReadConfig();

		std::string aob = "c7 ? ? 89 88 88 3c eb ? 89 ? 18 eb ? 89 ? 18 c7";
		std::string expectedBytes = "89 88 88 3c";
		std::string newBytes = "90 90 90 90";
		size_t offset = 3;

		float frametime = (1000 / fpsLimit) / 1000;
		Log("Frametime: ", frametime);
		std::vector<unsigned char> frametimeBytes(sizeof(float), 0);
		MemCopy((uintptr_t)&frametimeBytes[0], (uintptr_t)&frametime, 4);
		newBytes = RawAobToStringAob(frametimeBytes);

		uintptr_t patchAddress = AobScan(aob);
		if (patchAddress == 0)
		{
			return 1;
		}

		patchAddress += offset;
		if (!ReplaceExpectedBytesAtAddress(patchAddress, expectedBytes, newBytes))
		{
			return 1;
		}
	}

	Log("Removing 60 FPS fullscreen limit...");
	{
		std::string aob = "c7 ? ef 3c 00 00 00 c7 ? f3 01 00 00 00";
		std::string expectedBytes = aob;
		std::string newBytes = "c7 45 ef 00 00 00 00";
		uintptr_t patchAddress = AobScan(aob);
		if (patchAddress != 0)
		{
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