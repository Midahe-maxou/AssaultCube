
#define RE_EXT

#include <iostream>
#include "template.h"
#include "template_ext.h"
#include "AssaultCube.h"
#include "memmgr.h"


#define PROCNAME L"ac_client.exe"


int main()
{

	DWORD procId = getProcessId(PROCNAME);
	uintptr_t baseAddress = getModuleBaseAddress(procId, PROCNAME);


	init(procId);
	intptr_t playerBaseAddress = getAddressWithOffsetList(baseAddress + PLAYER_STATIC_OFFSET, PLAYER_OFFSET_LIST);
	Player player(playerBaseAddress);

	MemoryManager memmgr = MemoryManager(hProc);

	/*LargeUInt mask = 1;
	mask.printB();
	mask <<= 2;
	//mask -= 1;
	mask.printB();
	std::cout << mask.size() << std::endl;*/

	/*
	DWORD dwExit = 0;

	while (GetExitCodeProcess(gethProc(), &dwExit) && dwExit == STILL_ACTIVE)
	{
		if (GetAsyncKeyState(VK_NUMPAD0) & 1)
		{
			player.getCurrentWeapon()->setClip(100);
			std::cout << "Set clip to 100" << std::endl;
		}

		if (GetAsyncKeyState(VK_NUMPAD1) & 1)

		{
			player.getCurrentWeapon()->setAmmo(100);
		}

		if (GetAsyncKeyState(VK_NUMPAD2) & 1)
		{
			player.getCurrentWeapon()->setWeaponReloadLatency(100);
		}

		if (GetAsyncKeyState(VK_NUMPAD3) & 1)
		{
			player.getCurrentWeapon()->setWeaponShootLatency(100);
		}

		if (GetAsyncKeyState(VK_INSERT) & 1)
		{
			return 0;
		}

		Sleep(10);
		
	}
	*/
}
