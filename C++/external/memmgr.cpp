#include "memmgr.h"

#include <vector>
#include <array>
#include <stdexcept>
#include <list>

#include <memoryapi.h>
#include <Windows.h>
#include <sysinfoapi.h> // GetSystemInfo

//TODO: remove
#include <iostream>
#include <bitset>
#define bin(val) std::bitset<bsizeof(val)>(val)

DWORD pageSize = 0;
int nbInt64ForPage = 0;

void memmgrInit()
{
	SYSTEM_INFO sysInfos = SYSTEM_INFO();
	GetSystemInfo(&sysInfos);
	pageSize = sysInfos.dwPageSize;
	nbInt64ForPage = pageSize / bsizeof(int64_t);
}

_MemRepr::_MemRepr(_In_ DWORD pageAddress, _In_ HANDLE hProc, _In_opt_ bool debug)
	:m_pageAddress(pageAddress)
{
	if (!nbInt64ForPage) throw std::logic_error("Memory manager has not been initialized.");
	if (!pageAddress) throw std::invalid_argument("pageAddress should not be 0x0");
	if (pageAddress % pageSize != 0) throw std::invalid_argument("pageAddress should be the first address of the page");

	m_repr.reserve(nbInt64ForPage);

	if (!debug) return;

	std::array<BYTE, 64> allocatedData; // chunks of 64 bytes data.

	for (int i = 0; i < nbInt64ForPage; i++) {
		ReadProcessMemory(hProc, reinterpret_cast<void*>(pageAddress + i*64), allocatedData.data(), 64, NULL); // Read chunks of 64 bytes to save stack memory.

		for (BYTE byte : allocatedData)
		{
			m_repr[i] <<= 1; // Offset m_repr by 1 bit.
			m_repr[i] |= (BYTE)(bool)byte; // Set the m_repr bit to 0 if the byte is 0, or to 1 otherwise.
		}
	}
}


DWORD _MemRepr::FindSuitableMemory(_In_ size_t size, _In_ bool aligned)
{
	return NULL;
}

bool _MemRepr::ReserveMemory(_In_ DWORD address, _In_ size_t size)
{
	DWORD relativeAddress = (DWORD)(m_pageAddress - address);
	if (relativeAddress > pageSize) return false; // If address is before of after (note: 64U - 65U > 64)

	
	/*
	unsigned int chunk = relativeAddress / 64U;
	unsigned offset = relativeAddress % 64U;

	unsigned int nbChunks = size / 64U;
	*/


	return true;
}

void _MemRepr::PrintMemoryRepresentation()
{
	for (int i = 0; i < 64; i++)
	{
		std::cout << std::bitset<64>(m_repr[i]);
	}
}


MemoryManager::MemoryManager(HANDLE hProc)
	:m_hProc(hProc)
{}

DWORD MemoryManager::insertData(_In_ std::vector<BYTE> data, _In_opt_ bool aligned)
{
	size_t size = data.size();
	if (size > 4095) return (DWORD)0;
	DWORD address;
	for(auto it = std::rbegin(m_allocatedMem); it != std::rend(m_allocatedMem); it++)
	{
		address = it->FindSuitableMemory(size, aligned);
		if (address) return address;
	}

	address = reinterpret_cast<DWORD>(VirtualAllocEx(m_hProc, nullptr, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
	if (!address) return (DWORD)0;

	m_allocatedMem.push_back({ address, m_hProc });
	return address;
}

DWORD MemoryManager::insertData(_In_ BYTE data)
{
	std::vector<BYTE> d = { data };
	return this->insertData(d);
}

/**
 * @brief Return the number of 0's at the front.
*/
unsigned int CalculateMemOffset(uint64_t mask)
{
	if (mask == 0) return 64U;
	unsigned int zeros = 0;
	while ((mask & 1) == 0x0)
	{
		zeros += 1;
		mask >>= 1;
	}
	return zeros;
}