#include <thread>
#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(5s);
	
	Log("Activating ultrawide fix...");
	std::string aob = "48 c7 45 b8 fe ff ff ff 48 89 58 10 48 89 70 18 48 89 78 20 0f 29 70 c8 48 8b";
	std::string expectedBytes = "74";
	std::string newBytes = "eb";
	uintptr_t patchAddress = AobScan(aob);
	size_t offset = 0x94;
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