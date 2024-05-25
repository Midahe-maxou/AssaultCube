#pragma once
#ifndef MEMMGR_H
#define MEMMGR_H

#include <vector>
#include <list> // Double-linked list
#include <array>

#include <Windows.h>


#define bsizeof(val) (sizeof(val)*8)

//TODO: remove
#include <iostream>
#include <bitset>
#define bin(val) std::bitset<sizeof(val)*8>(val)



void memmgrInit(); // Must be called first.
unsigned int CalculateMemOffset(uint64_t mask);

class LargeUInt
{
private:
	std::vector<uint64_t> m_int{ 0 };

public:
	LargeUInt() noexcept = default;
	LargeUInt(std::vector<uint64_t> vect) noexcept
		:m_int(vect) {};
	LargeUInt(uint64_t i) noexcept
		:m_int({ i }) {};

	inline size_t size() const { return m_int.size(); }
	inline void resize(size_t size);
	void expand(size_t newSize);

	bool operator==(const LargeUInt& other) const noexcept;
	bool operator==(const uint64_t other) const noexcept;
	bool operator!=(const LargeUInt& other) const noexcept;

	LargeUInt& operator=(const LargeUInt& other) noexcept;
	LargeUInt& operator=(uint64_t other) noexcept;

	LargeUInt operator+(const LargeUInt& other) const noexcept;
	LargeUInt& operator+=(const LargeUInt& other) noexcept;
	LargeUInt operator+(uint64_t other) const noexcept;
	LargeUInt& operator+=(uint64_t other) noexcept;

	LargeUInt operator-(const LargeUInt& other) const noexcept;
	LargeUInt& operator-=(const LargeUInt& other) noexcept;
	LargeUInt operator-(uint64_t other) const noexcept;
	LargeUInt& operator-=(uint64_t other) noexcept;


	LargeUInt operator|(const LargeUInt& other) const noexcept;
	LargeUInt& operator|=(const LargeUInt& other) noexcept;
	LargeUInt operator|(uint64_t other) const noexcept;
	LargeUInt& operator|=(uint64_t other) noexcept;

	LargeUInt operator&(const LargeUInt& other) const noexcept;
	LargeUInt& operator&=(const LargeUInt& other) noexcept;
	LargeUInt operator&(uint64_t other) const noexcept;
	LargeUInt& operator&=(uint64_t other) noexcept;

	LargeUInt operator<<(UINT shift) const noexcept;
	LargeUInt& operator<<=(UINT shift) noexcept;
	LargeUInt operator>>(UINT shift) const noexcept;
	LargeUInt& operator>>=(UINT shift) noexcept;

	LargeUInt operator~() const noexcept;

public: static LargeUInt createMask(size_t size);


	//TODO: remove
	void print()
	{
		for (uint64_t i : m_int)
			std::cout << i << " ";
		std::cout << std::endl;
	}
	void printB()
	{
		for (uint64_t i : m_int)
		{
			if (i == 0) std::cout << 0 << " ";
			else std::cout << bin(i) << " ";
		}
		std::cout << std::endl;
	}
	void printH()
	{
		for (uint64_t i : m_int)
			std::cout << std::hex << i << " ";
		std::cout << std::endl;
	}

};

class _MemRepr
{
private:
	LargeUInt m_repr; // Will depends on the page size.
	DWORD m_pageAddress; // Address of the page. Must be a multiple of the page size.

public:
	_MemRepr(_In_ DWORD pageAddress, _In_ HANDLE hProc, _In_opt_ bool debug = false);
	DWORD FindSuitableMemory(_In_ size_t size, _In_ bool aligned = false) const;
	bool ReserveMemory(_In_ DWORD address, _In_ size_t size);
	bool FreeMemory(_In_ DWORD address, _In_ size_t size);
	void PrintMemoryRepresentation();
};

class MemoryManager
{
private:
	std::list<_MemRepr> m_allocatedMem; // _MemRepr could be too heavy to be in a vector, depending of the page size.
	HANDLE m_hProc;

public:
	MemoryManager(HANDLE hProc);
	DWORD insertData(_In_ BYTE data);
	DWORD insertData(_In_ std::vector<BYTE> data, _In_opt_ bool aligned = false);
};

#endif //MEMMGR_H