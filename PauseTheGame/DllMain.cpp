#include <Windows.h>
#include <xinput.h>

#include "ModUtils.h"

using namespace ModUtils;
using namespace mINI;

static bool gamePaused = false;
static uintptr_t patchAddress = 0;
static unsigned int key = 0x50;
static unsigned int modifier = HK_NONE;
static unsigned int controllerKey = 0x0010;
static unsigned int controllerModifier = HK_NONE;

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

std::string HexToString(uintptr_t hex)
{
	std::ostringstream stream;
	stream << "0x" << std::hex << hex;
	return stream.str();
}

uintptr_t HexStringToHex(std::string hexString)
{
	std::istringstream stream(hexString);
	uintptr_t hex = 0;
	stream >> std::hex >> hex;
	return hex;
}

unsigned int FilterKeyValue(std::string hexString, unsigned int defaultValue)
{
	if (hexString.length() > 2 && hexString[0] == '0' && hexString[1] == 'x')
	{
		hexString = hexString.substr(2, hexString.length());
		return HexStringToHex(hexString);
	}
	else if(!hexString.empty())
	{
		RaiseError("Invalid hex value in config file: '" + hexString + "'! Format: 0x<Number>. Using default keybind.");
	}
	return defaultValue;
}

void ReadConfig()
{
	INIFile config(GetModuleFolderPath() + "\\pause_keybind.ini");
	INIStructure ini;

	if (config.read(ini))
	{
		key = FilterKeyValue(ini["keyboard"].get("key1"), key);
		modifier = FilterKeyValue(ini["keyboard"].get("key2"), modifier);
		controllerKey = FilterKeyValue(ini["controller"].get("key1"), controllerKey);
		controllerModifier = FilterKeyValue(ini["controller"].get("key2"), controllerModifier);
	}
	else
	{
		ini["keyboard"]["key1"] = HexToString(key);
		ini["keyboard"]["key2"] = HexToString(modifier);
		ini["controller"]["key1"] = HexToString(controllerKey);
		ini["controller"]["key2"] = HexToString(controllerModifier);
		config.write(ini, true);
	}

	Log("key: 0x%x", key);
	Log("modifier: 0x%x", modifier);
	Log("controllerKey: 0x%x", controllerKey);
	Log("controllerModifier: 0x%x", controllerModifier);
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating PauseTheGame...");
	std::vector<uint16_t> pattern = { 0x0f, 0x84, MASKED, MASKED, MASKED, MASKED, 0xc6, MASKED, MASKED, MASKED, MASKED, MASKED, MASKED, MASKED, 0x8d, MASKED, MASKED, MASKED, MASKED, MASKED, MASKED, 0x89, MASKED, MASKED, 0x89, MASKED, MASKED, MASKED, 0x8b, MASKED, MASKED, MASKED, MASKED, MASKED, MASKED, 0x85, MASKED, 0x75 };
	patchAddress = SigScan(pattern);
	if (patchAddress == 0)
	{
		return 1;
	}

	ReadConfig();

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