#include <thread>
#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(5s);
	
	Log("Activating RemoveChromaticAberration...");
	std::string aob = "0f 11 ? 60 ? 8d ? 80 00 00 00 0f 10 ? a0 00 00 00 0f 11 ? f0 ? 8d ? b0 00 00 00 0f 10 ? 0f 11 ? 0f 10 ? 10";
	std::string expectedBytes = "0f 11 ? ?";
	std::string newBytes = "66 0f ef c9";
	uintptr_t patchAddress = AobScan(aob);
	size_t offset = 0x2f;
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