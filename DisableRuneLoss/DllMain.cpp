#include <thread>
#include <Windows.h>

#include "ModUtils.h"

using namespace ModUtils;

DWORD WINAPI MainThread(LPVOID lpParam)
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(5s);
	
	Log("Activating DisableRuneLoss...");
	std::vector<uint16_t> pattern = { 0xb0, 0x01, MASKED, 0x8b, MASKED, 0xe8, MASKED, MASKED, MASKED, MASKED, MASKED, 0x8b, MASKED, MASKED, MASKED, 0x32, 0xc0, MASKED, 0x83, MASKED, 0x28, 0xc3 };
	std::vector<uint16_t> originalBytes = { 0xe8 };
	std::vector<uint8_t> newBytes = { 0x90, 0x90, 0x90, 0x90, 0x90 };
	uintptr_t patchAddress = SigScan(pattern);
	if (patchAddress != 0)
	{
		patchAddress += 5;
		Replace(patchAddress, originalBytes, newBytes);
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