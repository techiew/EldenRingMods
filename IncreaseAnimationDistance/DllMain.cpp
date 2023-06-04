#include <thread>
#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(5s);
	
	Log("Activating IncreaseAnimationDistance...");
	std::string aob = "c7 ? ? ? 01 00 00 00 f3 ? 0f 10 ? ? ? f3 ? 0f 10 ? ? ? f3 0f 59 ? ? ? ? ? ? 0f 28 ? f3 ? 0f 5c ? ? 58";
	std::string expectedBytes = "f3 ? 0f 5e ? ? ?";
	std::string newBytes = "0f 57 c9 90 90 90 90";
	uintptr_t patchAddress = AobScan(aob);
	size_t offset = 0x48;

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