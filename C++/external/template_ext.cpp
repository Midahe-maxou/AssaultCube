#include "template_ext.h"
#ifdef RE_EXT

#include <Windows.h>
#include <memoryapi.h>
#include <vector>


int init(_In_ int procId)
{
	hProc = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
	return 0;
}

HANDLE gethProc()
{
	return hProc;
}

const uintptr_t getAddressWithOffsetList(_In_ uintptr_t address, _In_opt_ OFFSET_LIST offsetList)
{
	return ReadMemoryWithOffsetList<uintptr_t>(address, offsetList);
}

uintptr_t AllocateAndInjectCode(_In_ const std::vector<BYTE>& code, _In_ uintptr_t injectionAddress, _In_ BYTE opcodeLenght)
{
	if (opcodeLenght < 5) return NULL;

	uintptr_t allocatedAddress = reinterpret_cast<uintptr_t>(VirtualAllocEx(gethProc(), nullptr, 2048, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
	if (!allocatedAddress) return NULL;

	int jumpLength = allocatedAddress - injectionAddress;

	std::vector<BYTE> jmpCode =
	{
		0xE9		// jmp
	};

	concat(jmpCode, revAddressToBytes(jumpLength - 0x5UL + (uintptr_t)(opcodeLenght))); // Allocated address relative to injection address (+ reserved bytes).
	concat(jmpCode, getNops(opcodeLenght - 5)); // NOPs to fill the remaining bytes.

	std::vector<BYTE> initialCode = ReadCode(injectionAddress, 0, opcodeLenght); // Retreive initial code
	WriteCode(injectionAddress, 0, jmpCode);

	WriteCode(allocatedAddress, 0, initialCode); // Save initial code into reserved bytes.
	WriteCode(allocatedAddress, opcodeLenght, code); // Write injection code.

	jmpCode =
	{
		0xE9		// jmp
	};
	concat(jmpCode, revAddressToBytes((uintptr_t)(-(long)jumpLength) - code.size() - 0x5UL));	// Injection address relative to the end of injected code (+ reserved bytes).

	WriteCode(allocatedAddress, code.size() + opcodeLenght, jmpCode);
	return allocatedAddress;
}


//TODO: remove
#include <iostream>

bool RetreiveAndDesallocateCode(_In_ uintptr_t injectedAddress, _In_ BYTE opcodeLenght)
{
	std::vector<BYTE> jmpCode = ReadCode(injectedAddress, 0, 5);

	std::cout << "Jump code: ";
	for (BYTE b : jmpCode)
		std::cout << std::hex << (int)b << " ";
	std::cout << std::endl;

	if (jmpCode[0] != 0xE9) return false;

	jmpCode.erase(jmpCode.cbegin());
	uintptr_t allocatedAddress = revBytesToAddress(jmpCode) + (injectedAddress + 0x5) - (uintptr_t)opcodeLenght;

	std::cout << "Allocated address: " << std::hex << allocatedAddress << std::endl;


	std::vector<BYTE> initialCode = ReadCode(allocatedAddress, 0, opcodeLenght);

	std::cout << "Initial code: ";
	for (BYTE b : initialCode)
		std::cout << std::hex << (int)b << " ";
	std::cout << std::endl;

	if (!VirtualFreeEx(gethProc(), reinterpret_cast<void*>(allocatedAddress), NULL, MEM_RELEASE)) return false;
	WriteCode(injectedAddress, 0, initialCode);
	return true;
}

std::vector<BYTE> ReadCode(_In_ uintptr_t address, _In_ uintptr_t offset)
{
	std::vector<BYTE> dest;
	ReadProcessMemory(gethProc(), reinterpret_cast<void*>(address + offset), dest.data(), dest.size(), NULL);
	return dest;
}

std::vector<BYTE> ReadCode(_In_ uintptr_t address, _In_ uintptr_t offset, _In_ size_t size)
{
	std::vector<BYTE> dest;
	ReadProcessMemory(gethProc(), reinterpret_cast<void*>(address + offset), dest.data(), size, NULL);
	return dest;
}

bool WriteCode(_In_ uintptr_t address, _In_ uintptr_t offset, _In_ std::vector<BYTE> code)
{
	return WriteProcessMemory(gethProc(), reinterpret_cast<void*>(address + offset), code.data(), code.size(), NULL);
}

std::vector<BYTE> revAddressToBytes(_In_ uintptr_t address)
{
	std::vector<BYTE> bytes;

	for (int i = 0; i < 4; i++)
		bytes.push_back(((address >> (i * 8)) & 0xFF));

	return bytes;
}

uintptr_t revBytesToAddress(_In_ std::vector<BYTE> bytes)
{
	if (bytes.size() != 4) return NULL;
	uintptr_t address = NULL;

	for (int i = 0; i < 4; i++)
		address += bytes[i] << (i * 8);
	return address;
}

void concat(_In_ std::vector<BYTE>& vec1, _In_ const std::vector<BYTE>& vec2)
{
	vec1.reserve(vec2.size());
	vec1.insert(vec1.cend(), vec2.cbegin(), vec2.cend());
}

std::vector<BYTE> getNops(_In_ int numOfNop)
{
	if (numOfNop < 0) return {}; // Justin Case

	switch (numOfNop)
	{
	case 0:
		return {};
	case 1:
		return NOP1;
	case 2:
		return NOP2;
	case 3:
		return NOP3;
	case 4:
		return NOP4;
	case 5:
		return NOP5;
	case 6:
		return NOP6;
	case 7:
		return NOP7;
	case 8:
		return NOP8;

	default:
		std::vector<BYTE> nops = getNops(numOfNop - 8);
		nops.insert(nops.cbegin(), NOP8);
		return nops;
	}
}

#endif // RE_EXT