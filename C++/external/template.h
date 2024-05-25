#pragma once
#ifndef RE_TEMPLATE
#define RE_TEMPLATE


// TODO: remove

#define RE_EXT 
//#define RE_INT


#include <Windows.h>
#include <TlHelp32.h>
#include <vector>


// Multibytes nop.

#define NOP 0x90
#define NOP1 { 0x90 }
#define NOP2 { 0x66, 0x90 }
#define NOP3 { 0x0F, 0x1F, 0x00 }
#define NOP4 { 0x0F, 0x1F, 0x40, 0x00 }
#define NOP5 { 0x0F, 0x1F, 0x44, 0x00, 0x00 }
#define NOP6 { 0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00 }
#define NOP7 { 0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00 }
#define NOP8 { 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 }

typedef unsigned int OFFSET;
typedef std::vector<OFFSET> OFFSET_LIST;


struct Vector2I { int x = 0, y = 0; };
struct Vector3I { int x = 0, y = 0, z = 0; };

struct Vector2UI { unsigned int x = 0, y = 0; };
struct Vector3UI { unsigned int x = 0, y = 0, z = 0; };

struct Vector2F { float x = 0, y = 0; };
struct Vector3F { float x = 0, y = 0, z = 0; };



DWORD getProcessId(_In_ const TCHAR* procName);
MODULEENTRY32 getModuleEntry(_In_ DWORD procId, _In_ const TCHAR* modName);
HANDLE getHProcess(_In_ DWORD procId);
uintptr_t getModuleBaseAddress(_In_ DWORD procId, _In_ const TCHAR* modName);
size_t getModuleSize(_In_ DWORD procId, _In_ const TCHAR* modName);


class Pointer
{
protected:
	uintptr_t addressPtr = NULL;

public:
	Pointer(_In_ uintptr_t address)
		:addressPtr(address) {}

	uintptr_t getAddressPtr() const { return addressPtr; }

protected:
	Pointer() = default;
};




#endif // RE_TEMPLATE