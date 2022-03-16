#pragma once

#include <Windows.h>
#include <string>
#include <cstdarg>
#include <fileapi.h>
#include <Psapi.h>
#include <iostream>
#include <vector>

namespace ModUtils
{
	static std::string muModuleName = "";
	static FILE* logFile = nullptr;
	static const int MASKED = 0x00;

	inline std::string GetModuleName()
	{
		static char dummy = 'x';
		HMODULE module = NULL;
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, &dummy, &module);
		char lpFilename[MAX_PATH];
		GetModuleFileNameA(module, lpFilename, sizeof(lpFilename));
		std::string moduleName = strrchr(lpFilename, '\\');
		moduleName = moduleName.substr(1, moduleName.length());
		moduleName.erase(moduleName.find(".dll"), moduleName.length());
		return moduleName;
	}

	inline void Log(std::string msg, ...)
	{
		if (muModuleName == "")
		{
			muModuleName = GetModuleName();
		}

		if (logFile == nullptr)
		{
			CreateDirectoryA(std::string("mods\\" + muModuleName).c_str(), NULL);
			fopen_s(&logFile, std::string("mods\\" + muModuleName + "\\log.txt").c_str(), "w");
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

	inline void RaiseError(std::string error)
	{
		if (muModuleName == "")
		{
			muModuleName = GetModuleName();
		}
		Log("Raised error: %s", error.c_str());
		MessageBox(NULL, error.c_str(), muModuleName.c_str(), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
	}

	inline uintptr_t SigScan(std::vector<unsigned char> pattern, std::vector<unsigned char> mask = {})
	{
		MODULEINFO modInfo = { 0 };
		GetModuleInformation(GetCurrentProcess(), GetModuleHandle("eldenring.exe"), &modInfo, sizeof(MODULEINFO));
		uintptr_t currentAddress = (uintptr_t)modInfo.lpBaseOfDll;
		uintptr_t end = currentAddress + (uintptr_t)modInfo.SizeOfImage;

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
}