#pragma once
#ifndef MEMMGR_H
#define MEMMGR_H

#include <vector>
#include <list> // Double-linked list
#include <array>

#include <Windows.h>


#define bsizeof(val) sizeof(val)*8

//TODO: remove
#include <iostream>
#include <bitset>
#define bin(val) std::bitset<sizeof(val)*8>(val)



void memmgrInit(); // Must be called first.
unsigned int CalculateMemOffset(uint64_t mask);

class _MemRepr
{
private:
	std::vector<uint64_t> m_repr{ 0 }; // The vector's size will depends on the page size.
	DWORD m_pageAddress; // Address of the page. Must be a multiple of the page size.

public:
	_MemRepr(_In_ DWORD pageAddress, _In_ HANDLE hProc, _In_opt_ bool debug = false);
	DWORD FindSuitableMemory(_In_ size_t size, _In_ bool aligned = false);
	bool ReserveMemory(_In_ DWORD address, _In_ size_t size);
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


template<UINT I>
class MultiUInt64
{
private:
	std::array<uint64_t, I> m_int{ 0 };

public:
	MultiUInt64() noexcept = default;
	MultiUInt64(std::array<uint64_t, I> i) noexcept
		:m_int(i) {};
	//MultiUInt64(size_t size);

	inline size_t size() const noexcept { return I; }


	bool operator==(const MultiUInt64& other) const noexcept;
	MultiUInt64& operator|(const MultiUInt64& other) noexcept;
	MultiUInt64& operator&(const MultiUInt64& other) noexcept;


	MultiUInt64& operator<<(UINT shift) noexcept;
	MultiUInt64& operator>>(UINT shift) noexcept;


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
			std::cout << bin(i) << " ";
		std::cout << std::endl;
	}

};




// Template methods definitions

template<UINT I>
bool MultiUInt64<I>::operator==(const MultiUInt64<I>& other) const noexcept
{
	if (!I) return true;

	for (UINT i = 0; i < this->size(); i++)
		if (m_int[i] != other.m_int[i]) return false;

	return true;
}

template<UINT I>
MultiUInt64<I>& MultiUInt64<I>::operator<<(UINT shift) noexcept
{
	if (!I) return *this;

	int nbMovedChunk = shift / 64U;
	int offset = shift % 64U;

	if (nbMovedChunk >= I) // shift so high everything's 0.
	{
		m_int.fill(0);
		return *this;
	}

	if (nbMovedChunk)
	{
		for (int i = nbMovedChunk; i < I; i++)
			m_int[i - nbMovedChunk] = m_int[i]; // Move chunks by nbMovedChunk to the left.

		for (int i = 1; i <= nbMovedChunk; i++)
			m_int[I - i] = 0;
	}

	uint64_t temp = 0;
	m_int[0] <<= offset;
	for (UINT i = 1; i < I; i++)
	{
		temp = m_int[i];
		m_int[i] <<= offset;
		temp >>= bsizeof(uint64_t) - offset;
		m_int[i - 1] |= temp;
	}
	return *this;
}


template<UINT I>
MultiUInt64<I>& MultiUInt64<I>::operator>>(UINT shift) noexcept
{
	if (!I) return *this;

	UINT nbMovedChunk = shift / 64U;
	UINT offset = shift % 64U;

	if (nbMovedChunk >= I) // shift so high everything's 0.
	{
		m_int.fill(0);
		return *this;
	}

	if (nbMovedChunk)
	{
		for (UINT i = I-1; i > nbMovedChunk; i--)
			m_int[i] = m_int[i - nbMovedChunk]; // Move chunks by nbMovedChunk to the right.

		for (UINT i = 0; i <= nbMovedChunk; i++)
			m_int[i] = 0;
	}

	uint64_t temp = 0;
	m_int[I-1] >>= offset;
	for (int i = I-2; i >= 0; i--)
	{
		temp = m_int[i];
		m_int[i] >>= offset;
		temp <<= bsizeof(uint64_t) - offset;
		m_int[i + 1] |= temp;
	}
	return *this;
}

template<UINT I>
MultiUInt64<I>& MultiUInt64<I>::operator|(const MultiUInt64<I>& other) noexcept
{
	if (!I) return *this;

	for (UINT i = 0; i < I; i++)
		m_int[i] |= other.m_int[i];
	return *this;
}


template<UINT I>
MultiUInt64<I>& MultiUInt64<I>::operator&(const MultiUInt64<I>& other) noexcept
{
	if (!I) return *this;

	for (UINT i = 0; i < I; i++)
		m_int[i] &= other.m_int[i];
	return *this;
}


#endif //MEMMGR_H