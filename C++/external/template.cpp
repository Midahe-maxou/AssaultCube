#include "template.h"

#include <vector>


DWORD getProcessId(_In_ const TCHAR* procName)
{
	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (!hSnap || hSnap == INVALID_HANDLE_VALUE) return NULL;

	PROCESSENTRY32 procEntry = PROCESSENTRY32();
	procEntry.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnap, &procEntry))
	{
		do
		{
			if (_wcsicmp(procEntry.szExeFile, procName) == 0)
			{
				procId = procEntry.th32ProcessID;
				break;
			}
		} while (Process32Next(hSnap, &procEntry));
	}
	CloseHandle(hSnap);
	return procId;
}

MODULEENTRY32 getModuleEntry(_In_ DWORD procId, _In_ const TCHAR* modName)
{
	MODULEENTRY32 modEntry = MODULEENTRY32();
	modEntry.dwSize = 0;

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (!hSnap || hSnap == INVALID_HANDLE_VALUE) return modEntry;

	modEntry.dwSize = sizeof(MODULEENTRY32);


	if (Module32First(hSnap, &modEntry))
	{
		do
		{
			if (_wcsicmp(modEntry.szModule, modName) == 0)
				break;
		} while (Module32Next(hSnap, &modEntry));
	}

	CloseHandle(hSnap);
	return modEntry;
}

HANDLE getHProcess(_In_ DWORD procId)
{
	return OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
}

uintptr_t getModuleBaseAddress(_In_ DWORD procId, _In_ const TCHAR* modName)
{
	MODULEENTRY32 modEntry = getModuleEntry(procId, modName);
	if(modEntry.dwSize == 0)
		return NULL;
	return (uintptr_t)modEntry.modBaseAddr;
}

size_t getModuleSize(_In_ DWORD procId, _In_ const TCHAR* modName)
{
	MODULEENTRY32 modEntry = getModuleEntry(procId, modName);
	if (modEntry.dwSize == 0)
		return 0;
	return (size_t)modEntry.modBaseSize;
}
