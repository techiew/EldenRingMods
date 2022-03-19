#include <Windows.h>
#include <xinput.h>

#include "ModUtils.h"
#include "ini.h"

using namespace ModUtils;
using namespace mINI;

static bool gamePaused = false;
static uintptr_t patchAddress = 0;
static WORD key = 0x50;
static WORD modifier = HK_NONE;
static WORD controllerKey = 0x0010;
static WORD controllerModifier = HK_NONE;

void TogglePause()
{
	if (gamePaused)
	{
		Log("Unpaused");
		Replace(patchAddress + 1, { 0x85 }, { 0x84 });
		gamePaused = false;
	}
	else
	{
		Log("Paused");
		Replace(patchAddress + 1, { 0x84 }, { 0x85 });
		gamePaused = true;
	}
}

void ReadIniFile()
{
	INIFile config(GetModuleFolderPath() + "\\pause_keybind.ini");
	INIStructure ini;
	bool readSuccess = config.read(ini);
	if (readSuccess)
	{
		std::stringstream key1(ini["keyboard"]["key1"]);
		std::stringstream key2(ini["keyboard"]["key2"]);
		std::stringstream controllerKey1(ini["controller"]["key1"]);
		std::stringstream controllerKey2(ini["controller"]["key2"]);

		// Avoid setting the key values if the config has not been changed,
		// this might be a temporary workaround for people who have had issues.
		// I guess we'll see.
		if (key1.str() != "0x50")
		{
			key1 >> std::hex >> key;
			Log("New key1");
		}

		if (key2.str() != "0x07")
		{
			key2 >> std::hex >> modifier;
			Log("New key2");
		}

		if (controllerKey1.str() != "0x0010")
		{
			controllerKey1 >> std::hex >> controllerKey;
			Log("New controllerKey1");
		}

		if (controllerKey2.str() != "0x07")
		{
			controllerKey2 >> std::hex >> controllerModifier;
			Log("New controllerKey2");
		}

		Log("key1: %i", key);
		Log("key2: %i", modifier);
		Log("controllerKey1: %i", controllerKey);
		Log("controllerKey2: %i", controllerModifier);
	}
	else
	{
		ini["keyboard"]["key1"] = "0x50";
		ini["keyboard"]["key2"] = "0x07";
		ini["controller"]["key1"] = "0x0010";
		ini["controller"]["key2"] = "0x07";
		config.generate(ini, true);
		Log("Created new .ini");
	}
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating PauseTheGame...");
	std::vector<uint16_t> pattern = { 0x0f, 0x84, MASKED, MASKED, MASKED, MASKED, 0xc6, 0x83, MASKED, MASKED, 0x00, 0x00, 0x00, 0x48, 0x8d, MASKED, MASKED, MASKED, MASKED, MASKED, 0x48, 0x89, MASKED, MASKED, 0x89 };
	patchAddress = SigScan(pattern);
	if (patchAddress == 0)
	{
		return 1;
	}

	ReadIniFile();

	while (true)
	{
		if (CheckHotkey(key, modifier) || CheckHotkey(controllerKey, controllerModifier, true))
		{
			TogglePause();
		}
		Sleep(5);
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