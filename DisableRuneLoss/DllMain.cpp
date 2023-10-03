#include <thread>
#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(5s);
	
	Log("Activating DisableRuneLoss...");
	std::string aob = "b0 01 ? 8b ? e8 ? ? ? ? ? 8b ? ? ? 32 c0 ? 83 ? 28 c3";
	std::string expectedBytes = "e8";
	std::string newBytes = "90 90 90 90 90";
	uintptr_t patchAddress = AobScan(aob);
	size_t offset = 5;

	if (patchAddress != 0)
	{
		patchAddress += offset;
		ReplaceExpectedBytesAtAddress(patchAddress, expectedBytes, newBytes);
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