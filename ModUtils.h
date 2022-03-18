#pragma once

#include <Windows.h>
#include <string>
#include <cstdarg>
#include <fileapi.h>
#include <Psapi.h>
#include <iostream>
#include <vector>
#include <xinput.h>

namespace ModUtils
{
	static std::string muModuleName = "";
	static FILE* logFile = nullptr;
	static bool logOpened = false;
	static bool enumWindowsCalled = false;
	static const int MASKED = 0x00;
	static HWND muWindow = NULL;
	constexpr unsigned char HK_NONE = 0x07;

	inline std::string GetModuleName(bool thisModule)
	{
		static char dummy = 'x';
		HMODULE module = NULL;

		if (thisModule)
		{
			GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, &dummy, &module);
		}

		char lpFilename[MAX_PATH];
		GetModuleFileNameA(module, lpFilename, sizeof(lpFilename));
		std::string moduleName = strrchr(lpFilename, '\\');
		moduleName = moduleName.substr(1, moduleName.length());

		if (thisModule)
		{
			moduleName.erase(moduleName.find(".dll"), moduleName.length());
		}

		return moduleName;
	}

	inline std::string GetModuleFolderPath()
	{
		return std::string("mods\\" + GetModuleName(true));
	}

	inline void Log(std::string msg, ...)
	{
		if (muModuleName == "")
		{
			muModuleName = GetModuleName(true);
		}

		if (logFile == nullptr && !logOpened)
		{
			CreateDirectoryA(std::string("mods\\" + muModuleName).c_str(), NULL);
			fopen_s(&logFile, std::string("mods\\" + muModuleName + "\\log.txt").c_str(), "w");
			logOpened = true;
		}

		va_list args;
		va_start(args, msg);
		vprintf(std::string(muModuleName + " > " + msg + "\n").c_str(), args);
		if (logFile != nullptr)
		{
			vfprintf(logFile, std::string(muModuleName + " > " + msg + "\n").c_str(), args);
			fflush(logFile);
		}
		va_end(args);
	}

	inline void CloseLog()
	{
		if (logFile != nullptr)
		{
			fclose(logFile);
			logFile = nullptr;
		}
	}

	inline void RaiseError(std::string error)
	{
		if (muModuleName == "")
		{
			muModuleName = GetModuleName(true);
		}
		Log("Raised error: %s", error.c_str());
		MessageBox(NULL, error.c_str(), muModuleName.c_str(), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
	}

	inline uintptr_t SigScan(std::vector<unsigned char> pattern, std::vector<unsigned char> mask = {})
	{
		MODULEINFO modInfo = { 0 };
		std::string moduleName = GetModuleName(false);
		GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(moduleName.c_str()), &modInfo, sizeof(MODULEINFO));
		uintptr_t currentAddress = (uintptr_t)modInfo.lpBaseOfDll;
		uintptr_t end = currentAddress + (uintptr_t)modInfo.SizeOfImage;
		Log("Module: %s = %p", moduleName.c_str(), currentAddress);

		size_t numRegionsChecked = 0;
		MEMORY_BASIC_INFORMATION memoryInfo;
		while (currentAddress < end)
		{
			VirtualQuery((void*)currentAddress, &memoryInfo, sizeof(MEMORY_BASIC_INFORMATION));
			uintptr_t regionStart = (uintptr_t)memoryInfo.BaseAddress;
			uintptr_t regionSize = (uintptr_t)memoryInfo.RegionSize;
			uintptr_t regionEnd = regionStart + regionSize;
			uintptr_t protection = (uintptr_t)memoryInfo.Protect;
			uintptr_t state = (uintptr_t)memoryInfo.State;

			if ((protection == PAGE_EXECUTE_READWRITE || protection == PAGE_READWRITE) && state == MEM_COMMIT)
			{
				if (numRegionsChecked > 2000)
				{
					break;
				}

				Log("Checking region: %p", memoryInfo.BaseAddress);
				while (currentAddress < regionEnd - pattern.size())
				{
					for (size_t i = 0; i < pattern.size(); i++)
					{
						if (std::find(mask.begin(), mask.end(), i) != mask.end())
						{
							currentAddress++;
							continue;
						}
						else if (*(unsigned char*)currentAddress != pattern[i])
						{
							currentAddress++;
							break;
						}
						else if (i == pattern.size() - 1)
						{
							uintptr_t signature = currentAddress - pattern.size() + 1;
							Log("Found signature at %p", signature);
							return signature;
						}

						currentAddress++;
					}
				}
				numRegionsChecked++;
			}
			else
			{
				currentAddress += memoryInfo.RegionSize;
			}
		}
		RaiseError("Could not find signature!");
		return 0;
	}

	inline bool Replace(uintptr_t address, std::vector<unsigned char> originalBytes, std::vector<unsigned char> newBytes, std::vector<unsigned char> mask = {})
	{
		std::vector<unsigned char> buffer(originalBytes.size());
		memcpy(&buffer[0], (void*)address, buffer.size());
		for (size_t index : mask)
		{
			if (index >= buffer.size())
			{
				RaiseError("Mask goes out of bounds!");
				return false;
			}
			buffer[index] = MASKED;
		}

		if (memcmp(&buffer[0], &originalBytes[0], buffer.size()) == 0)
		{
			Log("Bytes match");
			memcpy((void*)address, &newBytes[0], newBytes.size());
			Log("Patch applied");
			return true;
		}
		else
		{
			RaiseError("Bytes do not match!");
		}
		return false;
	}

	BOOL CALLBACK GetApplicationWindow(HWND hwnd, LPARAM lParam)
	{
		DWORD processId = NULL;
		GetWindowThreadProcessId(hwnd, &processId);
		if (processId != NULL)
		{
			if (processId == GetCurrentProcessId())
			{
				muWindow = hwnd;
				return false;
			}
		}
		return true;
	}

	inline bool CheckHotkey(WORD key, WORD modifier = HK_NONE, bool checkController = false)
	{
		static std::vector<unsigned int> notReleasedKeys;

		if (!enumWindowsCalled)
		{
			EnumWindows(&GetApplicationWindow, NULL);
			enumWindowsCalled = true;
		}

		if (muWindow != GetForegroundWindow())
		{
			return false;
		}

		bool keyPressed = false;
		bool modifierPressed = false;

		if (checkController)
		{
			for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; controllerIndex++)
			{
				XINPUT_STATE state;
				ZeroMemory(&state, sizeof(XINPUT_STATE));
				DWORD result = XInputGetState(controllerIndex, &state);
				if (result == ERROR_SUCCESS)
				{
					if (state.Gamepad.wButtons == key)
					{
						keyPressed = true;
					}
					else if (state.Gamepad.wButtons == modifier)
					{
						modifierPressed = true;
					}
					else if (state.Gamepad.wButtons == (key + modifier))
					{
						keyPressed = true;
						modifierPressed = true;
					}
				}
			}
		}
		else
		{
			keyPressed = GetAsyncKeyState(key) & 0x8000;
			modifierPressed = GetAsyncKeyState(modifier) & 0x8000;
		}

		if (key == HK_NONE)
		{
			return modifierPressed;
		}

		auto iterator = std::find(notReleasedKeys.begin(), notReleasedKeys.end(), key);
		bool keyNotReleased = iterator != notReleasedKeys.end();

		if (keyPressed && keyNotReleased)
		{
			return false;
		}

		if (!keyPressed)
		{
			if (keyNotReleased)
			{
				notReleasedKeys.erase(iterator);
			}
			return false;
		}

		if (modifier != HK_NONE && !modifierPressed)
		{
			return false;
		}

		notReleasedKeys.push_back(key);
		return true;
	}

	inline void Hook(uintptr_t address, uintptr_t destination, size_t clearance)
	{
		DWORD oldProtection;
		VirtualProtect((void*)address, clearance, PAGE_EXECUTE_READWRITE, &oldProtection);
		memset((void*)address, 0x90, clearance);
		*(uintptr_t*)address = 0x0000000025ff;
		memcpy((void*)(address + 6), (void*)&destination, 8);
		VirtualProtect((void*)address, clearance, oldProtection, &oldProtection);
	}
}