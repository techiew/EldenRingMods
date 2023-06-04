#include <thread>
#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(5s);
	
	Log("Activating RemoveVignette...");
	std::string aob = "f3 0f 10 ? 50 f3 0f 59 ? ? ? ? ? e8 ? ? ? ? f3 ? 0f 5c ? f3 ? 0f 59 ? ? 8d ? ? a0 00 00 00";
	std::string expectedBytes = "f3 ? 0f 59 ?";
	std::string newBytes = "f3 0f 5c c0 90";
	uintptr_t patchAddress = AobScan(aob);
	size_t offset = 0x17;
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