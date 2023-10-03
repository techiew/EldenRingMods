#include <thread>
#include <Windows.h>
#include <xinput.h>

#include "ModUtils.h"
#include "InputTranslation.h"

using namespace ModUtils;
using namespace mINI;

bool gameIsPaused = false;
uintptr_t patchAddress = 0;
size_t offset = 1;

struct Keybind
{
	std::vector<unsigned short> keys;
	bool isControllerKeybind;
};

std::vector<Keybind> pauseKeybinds = {
	{ { keycodes.at("p") }, false },
	{ { controllerKeycodes.at("lthumbpress"), controllerKeycodes.at("xa") }, true }
};
std::vector<Keybind> unpauseKeybinds = {
	{ { keycodes.at("p") }, false },
	{ { controllerKeycodes.at("lthumbpress"), controllerKeycodes.at("xa") }, true }
};

void Pause()
{
	Log("Paused");
	ReplaceExpectedBytesAtAddress(patchAddress + offset, "84", "85");
	gameIsPaused = true;
}

void Unpause()
{
	Log("Unpaused");
	ReplaceExpectedBytesAtAddress(patchAddress + offset, "85", "84");
	gameIsPaused = false;
}

std::vector<std::string> splitString(std::string str, std::string delimiter)
{
	size_t pos = 0;
	std::vector<std::string> list;
	while ((pos = str.find(delimiter)) != std::string::npos)
	{
		std::string token = str.substr(0, pos);
		list.push_back(token);
		str.erase(0, pos + delimiter.size());
	}
	list.push_back(str);
	return list;
}

std::vector<Keybind> TranslateInput(std::string inputString)
{
	std::vector<Keybind> keybinds;
	std::vector<std::vector<std::string>> keybindsToTranslate;

	// Remove spaces
	inputString.erase(std::remove_if(inputString.begin(), inputString.end(), std::isspace), inputString.end());

	// Convert to lowercase
	transform(inputString.begin(), inputString.end(), inputString.begin(), ::tolower);

	// Parse individual and combination keybinds and place in list
	std::vector<std::string> splitOnComma = splitString(inputString, ",");
	for (auto keybind : splitOnComma)
	{
		std::vector<std::string> splitOnPlus = splitString(keybind, "+");
		if (splitOnPlus.size() == 1)
		{
			keybindsToTranslate.push_back({ keybind });
		}
		else
		{
			std::vector<std::string> combos;
			for (auto combo : splitOnPlus)
			{
				combos.push_back(combo);
			}
			keybindsToTranslate.push_back(combos);
		}
	}

	// Convert raw keybind strings to keycodes
	for (auto rawKeybinds : keybindsToTranslate)
	{
		bool isControllerKeybind = false;
		std::vector<unsigned short> keybindGroup;
		for (std::string rawKeybindString : rawKeybinds)
		{
			auto search = keycodes.find(rawKeybindString);
			if (search != keycodes.end())
			{
				isControllerKeybind = false;
				keybindGroup.push_back(keycodes.at(rawKeybindString));
			}
			else
			{
				search = controllerKeycodes.find(rawKeybindString);
				if (search != controllerKeycodes.end())
				{
					isControllerKeybind = true;
					keybindGroup.push_back(controllerKeycodes.at(rawKeybindString));
				}
			}
		}
		keybinds.push_back({ keybindGroup, isControllerKeybind });
	}

	return keybinds;
}

void ReadConfig()
{
	INIFile config(GetModFolderPath() + "\\pause_keybinds.ini");
	INIStructure ini;

	if (config.read(ini))
	{
		pauseKeybinds = TranslateInput(ini["keybinds"].get("pause_keys"));
		unpauseKeybinds = TranslateInput(ini["keybinds"].get("unpause_keys"));
	}
	else
	{
		ini["keybinds"]["pause_keys"] = "p, lthumbpress+xa";
		ini["keybinds"]["unpause_keys"] = "p, lthumbpress+xa";
		config.write(ini, true);
	}
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(5s);
	
	Log("Activating PauseTheGame...");
	std::string aob = "0f 84 ? ? ? ? c6 ? ? ? ? ? 00 ? 8d ? ? ? ? ? ? 89 ? ? 89 ? ? ? 8b ? ? ? ? ? ? 85 ? 75";
	patchAddress = AobScan(aob);
	if (patchAddress == 0)
	{
		return 1;
	}

	ReadConfig();

	while (true)
	{
		auto* keybinds = &pauseKeybinds;
		if (gameIsPaused)
		{
			keybinds = &unpauseKeybinds;
		}

		for (Keybind keybind : *keybinds)
		{
			if (AreKeysPressed(keybind.keys, false, keybind.isControllerKeybind))
			{
				if (gameIsPaused)
				{
					Unpause();
				}
				else
				{
					Pause();
				}
			}
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