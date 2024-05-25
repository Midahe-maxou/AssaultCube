#include "memmgr.h"

#include <vector>
#include <array>
#include <stdexcept>
#include <list>
#include <cassert>

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

_MemRepr::_MemRepr(_In_ DWORD pageAddress,_In_ HANDLE hProc, _In_opt_ bool debug)
	:m_pageAddress(pageAddress)
{
	if (!nbInt64ForPage) throw std::logic_error("Memory manager has not been initialized.");
	if (!pageAddress) throw std::invalid_argument("pageAddress should not be 0x0");
	if (pageAddress % pageSize != 0) throw std::invalid_argument("pageAddress should be the first address of the page");

	m_repr.resize(nbInt64ForPage);

	if (!debug) return;

	std::array<BYTE, 64> allocatedData; // chunks of 64 bytes data.

	for (int i = 0; i < nbInt64ForPage; i++) {
		ReadProcessMemory(hProc, reinterpret_cast<void*>(pageAddress + i*64), allocatedData.data(), 64, NULL); // Read chunks of 64 bytes to save stack memory.

		for (BYTE byte : allocatedData)
		{
			m_repr <<= 1; // Offset m_repr by 1 bit.
			m_repr |= (BYTE)(bool)byte; // Set the m_repr bit to 0 if the byte is 0, or 1 otherwise.
		}
	}
}

/**
 * @brief Find an emplacement where data can fit.
 * 
 * @param size		The size of the data to write.
 * @param aligned	Weather the data should be aligned (alignment is done from the last byte of the page).
 * 
 * @note The research is done from the last byte to the first byte of the page.
 * 
 * @retval DWORD
 * @return The address where the data can be placed, or 0 if the data cannot be placed.
 */
DWORD _MemRepr::FindSuitableMemory(_In_ size_t size, _In_ bool aligned) const
{
	if (!size) return NULL;

	size_t nbChunk = size / 64U;

	size_t remains = size % 64U;

	LargeUInt mask = 1;
	mask <<= size;
	mask -= 1;

	int shift = 1;
	if (aligned) shift = size;

	for (int i = m_repr.size()*64; i > 0; i-=shift)
	{
		if ((m_repr & mask) == 0)
			// m_pageAddress is the address of the first byte of that page.
			// i is the offset where the last byte of the data should be.
			// Then negate the size to find the address of the 1st byte.
			return (m_pageAddress + i - size); 

		mask <<= shift;
	}

	return NULL;
}

/**
 * @brief Mark the data in the intern structure as reserved
 * 
 * @param address 
 * @param size 
 * 
 * @retval bool
 * @return false if the data is not contains in the page range, true otherwise.
 */
bool _MemRepr::ReserveMemory(_In_ DWORD address, _In_ size_t size)
{
	DWORD relativeAddress = m_pageAddress - address;
	if ((relativeAddress > pageSize) or (relativeAddress + size > pageSize)) return false; // If address is before of after the page.

	LargeUInt mask = LargeUInt::createMask(size);
	mask << (pageSize - relativeAddress); // Because the mask begin at the end, whereas the address begin at the begining.

	//TODO: remove -> can crash if the data is all 0's.
	assert((m_repr & mask) == 0);

	m_repr &= mask;
	
	return true;
}

/**
 * @brief Mark the data in the intern structure as free.
 *
 * @param address
 * @param size
 *
 * @retval bool
 * @return false if the data is not contains in the page range, true otherwise.
 */
bool _MemRepr::FreeMemory(_In_ DWORD address, _In_ size_t size)
{
	DWORD relativeAddress = m_pageAddress - address;
	if ((relativeAddress > pageSize) or (relativeAddress + size > pageSize)) return false; // If address is before of after the page.

	LargeUInt mask = LargeUInt::createMask(size);
	mask << (pageSize - relativeAddress); // Because the mask begin at the end, whereas the address begin at the begining.
	mask = ~mask;
	m_repr &= mask;

	return true;
}

void _MemRepr::PrintMemoryRepresentation()
{
	m_repr.printB();
}


MemoryManager::MemoryManager(HANDLE hProc)
	:m_hProc(hProc)
{
	memmgrInit();
}

DWORD MemoryManager::insertData(_In_ std::vector<BYTE> data, _In_opt_ bool aligned)
{
	size_t size = data.size();
	if (size > pageSize) return (DWORD)0;
	DWORD address;

	for (_MemRepr memRepr : m_allocatedMem)
	{
		if (address = memRepr.FindSuitableMemory(size, aligned))
		{
			memRepr.ReserveMemory(address, size);
			goto writeData;
		}
	}
//  else
	{
		address = reinterpret_cast<DWORD>(VirtualAllocEx(m_hProc, nullptr, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
		if (!address) return (DWORD)0;
		_MemRepr memRepr{ address, m_hProc };
		m_allocatedMem.push_back(memRepr);
		address = memRepr.FindSuitableMemory(size, aligned); // To satisfy weird alignement.
		memRepr.ReserveMemory(address, size);
	}

writeData:
	
	WriteProcessMemory(m_hProc, reinterpret_cast<void*>(address), data.data(), size, NULL);
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
	while ((mask & 1) == 0)
	{
		zeros += 1;
		mask >>= 1;
	}
	return zeros;
}


/**
 * @brief Resize the LargeUInt.
 * @param size The new size.
 * 
 * @note The values are invalidated after the resize. To keep all the values, use expand().
 * @note If the new size the greater than the actual size, 0's are added at the back.
 */
inline void LargeUInt::resize(size_t size)
{
	m_int.resize(size);
}

/**
 * @brief Expand the LargeUIt by adding 0's at the front.
 * @param newSize The size the LargeUInt should be.
 * 
 * @note If newSize is less than the actual size, this function has no effect. To force the resize, use resize().
 */
void LargeUInt::expand(size_t newSize)
{
	// Prevent information loss.
	int size = this->size();
	int diff = newSize - size;
	if (diff > 0)
	{
		this->resize(newSize);
		for (int i = size - 1; i >= 0; i--)
		{
			this->m_int[i + diff] = this->m_int[i]; // Move the data to the back (so we just add 0's at the front).
			this->m_int[i] = 0;
		}
		size = newSize;
	}
}


/**
 * @brief Check if two LargeUInt are equal.
 * @param other The other LargeUInt the instance is compared to.
 * 
 * @retval bool
 * @return True if the two LargeUInt are equals, false otherwise.
 */
bool LargeUInt::operator==(const LargeUInt& other) const noexcept
{
	return ((*this) & (~other)) == 0;
}

/**
 * @brief Check if the instance is equal to a uint64_t.
 * @param other The uint64_t the instance is compared to.
 * 
 * @retval bool
 * @return True is the intance is equal to the uint64_t, false otherwise.
 */
bool LargeUInt::operator==(const uint64_t other) const noexcept
{
	size_t size = this->size();
	for (int i = 0; i < size - 1; i++)
	{
		if (this->m_int[i] != 0) return false;
	}
	return this->m_int[size - 1] == other;
}

bool LargeUInt::operator!=(const LargeUInt& other) const noexcept
{
	return !(*this == other);
}

LargeUInt& LargeUInt::operator=(const LargeUInt& other) noexcept
{
	if (this == &other) return *this;

	this->m_int = other.m_int; // Vector::operator= copies the whole vector.
	return *this;
}

LargeUInt& LargeUInt::operator=(uint64_t other) noexcept
{
	std::vector<uint64_t> vect{ other };
	size_t size = this->size();
	this->m_int = vect;
	// Keep the initial vector size.
	this->expand(size);

	return *this;
}

LargeUInt LargeUInt::operator+(const LargeUInt& other) const noexcept
{
	size_t tSize = this->size();
	size_t oSize = other.size();

	LargeUInt larger = (tSize > oSize) ? *this : other;
	LargeUInt smaller = (tSize > oSize) ? other : *this;
	size_t diff = larger.size() - smaller.size();

	for (UINT i = 0; i < smaller.size(); i++)
		larger.m_int[i + diff] += smaller.m_int[i];

	return larger;
}

LargeUInt& LargeUInt::operator+=(const LargeUInt& other) noexcept
{
	*this = (*this) + other;
	return *this;
}

LargeUInt LargeUInt::operator+(uint64_t other) const noexcept
{
	LargeUInt cmp = *this;
	cmp.m_int[size() - 1] += other;
	return cmp;
}

LargeUInt& LargeUInt::operator+=(uint64_t other) noexcept
{
	*this = (*this) + other;
	return *this;
}

LargeUInt LargeUInt::operator-(const LargeUInt& other) const noexcept
{
	size_t tSize = this->size();
	size_t oSize = other.size();

	LargeUInt larger = (tSize > oSize) ? *this : other;
	LargeUInt smaller = (tSize > oSize) ? other : *this;
	size_t diff = larger.size() - smaller.size();

	for (UINT i = 0; i < smaller.size(); i++)
		larger.m_int[i + diff] -= smaller.m_int[i];

	return larger;
}

LargeUInt& LargeUInt::operator-=(const LargeUInt& other) noexcept
{
	*this = (*this) - other;
	return *this;
}

LargeUInt LargeUInt::operator-(uint64_t other) const noexcept
{
	LargeUInt cmp = *this;
	cmp.m_int[size() - 1] -= other;
	return cmp;
}

LargeUInt& LargeUInt::operator-=(uint64_t other) noexcept
{
	*this = (*this) - other;
	return *this;
}


LargeUInt LargeUInt::operator|(const LargeUInt& other) const noexcept
{
	size_t tSize = this->size();
	size_t oSize = other.size();

	LargeUInt larger = (tSize > oSize) ? *this : other;
	LargeUInt smaller = (tSize > oSize) ? other : *this;
	size_t diff = larger.size() - smaller.size();

	for (UINT i = 0; i < smaller.size(); i++)
		larger.m_int[i + diff] |= smaller.m_int[i];

	return larger;
}

LargeUInt& LargeUInt::operator|=(const LargeUInt& other) noexcept
{
	*this = (*this) | other;
	return *this;
}

LargeUInt LargeUInt::operator|(uint64_t other) const noexcept
{
	LargeUInt cmp = *this;
	cmp.m_int[size() - 1] |= other;
	return cmp;
}

LargeUInt& LargeUInt::operator|=(uint64_t other) noexcept
{
	*this = (*this) | other;
	return *this;
}


LargeUInt LargeUInt::operator&(const LargeUInt& other) const noexcept
{
	size_t tSize = this->size();
	size_t oSize = other.size();

	LargeUInt larger = (tSize > oSize) ? *this : other;
	LargeUInt smaller = (tSize > oSize) ? other : *this;

	size_t diff = larger.size() - smaller.size();

	// Set all digit that the smaller does not contain to 0 (because 1 & 0 == 0 & 0 == 0).
	for (int i = 0; i < diff; i++)
		larger.m_int[i] = 0;


	for (UINT i = 0; i < smaller.size(); i++)
		larger.m_int[i + diff] &= smaller.m_int[i];

	return larger;
}

LargeUInt& LargeUInt::operator&=(const LargeUInt& other) noexcept
{
	*this = (*this) & other;
	return *this;
}

LargeUInt LargeUInt::operator&(uint64_t other) const noexcept
{
	LargeUInt cmp = *this;
	cmp.m_int[size() - 1] &= other;
	return cmp;
}

LargeUInt& LargeUInt::operator&=(uint64_t other) noexcept
{
	*this = (*this) & other;
	return *this;
}

/**
 * @brief Perform a left bit shifting without losing any informations.
 * 
 * @param shift Number of bits to move to the left.
 * @return A LargeUInt bitshifted.
 * 
 * @note The size of the return LargeUInt is at least the size of the base LargeUInt, but it might be larger to avoid overflow.
 */
LargeUInt LargeUInt::operator<<(UINT shift) const noexcept
{
	size_t size = this->size();
	LargeUInt cmp = *this;

	int nbMovedChunk = shift / 64U;
	int offset = shift % 64U;

	// Memory usage optimisation.
	int nbEmptyChunk = 0;
	while (nbEmptyChunk < size and cmp.m_int[nbEmptyChunk] == 0)
		nbEmptyChunk += 1;

	// Prevent information loss.
	int newSize = nbMovedChunk + size - nbEmptyChunk;
	
	size = (size > newSize) ? size : newSize;
	size += 1; // +1 to prevent overflow.
	
	cmp.expand(size);

	if (nbMovedChunk)
	{
		for (int i = 0; i < size - nbMovedChunk; i++)
		{
			cmp.m_int[i] = cmp.m_int[i + nbMovedChunk]; // Move chunks by nbMovedChunk to the left.
			cmp.m_int[i + nbMovedChunk] = 0;
		}
	}

	uint64_t temp = 0;
	// Note: here, cmp.m_int[0] == 0.
	for (UINT i = 1; i < size; i++)
	{
		temp = cmp.m_int[i];
		cmp.m_int[i] <<= offset;
		temp >>= bsizeof(uint64_t) - offset;
		cmp.m_int[i - 1] |= temp;
	}

	// No overflow has occured.
	if (cmp.m_int[0] == 0)
		cmp.m_int.erase(cmp.m_int.begin());
	return cmp;
}


LargeUInt& LargeUInt::operator<<=(UINT shift) noexcept
{
	*this = (*this) << shift;
	return *this;
}

LargeUInt LargeUInt::operator>>(UINT shift) const noexcept
{
	size_t size = this->size();
	LargeUInt cmp = *this;

	if (!size) return cmp;

	UINT nbMovedChunk = shift / 64U;
	UINT offset = shift % 64U;

	// shift so high everything's 0.
	if (nbMovedChunk >= size)
	{
		cmp.m_int.assign(size, 0);
		return cmp;
	}

	if (nbMovedChunk)
	{
		for (UINT i = size - 1; i > nbMovedChunk; i--)
			cmp.m_int[i] = cmp.m_int[i - nbMovedChunk]; // Move chunks by nbMovedChunk to the right.

		for (UINT i = 0; i <= nbMovedChunk; i++)
			cmp.m_int[i] = 0;
	}

	uint64_t temp = 0;
	cmp.m_int[size - 1] >>= offset;
	for (int i = size - 2; i >= 0; i--)
	{
		temp = cmp.m_int[i];
		cmp.m_int[i] >>= offset;
		temp <<= bsizeof(uint64_t) - offset;
		cmp.m_int[i + 1] |= temp;
	}
	return cmp;
}

LargeUInt& LargeUInt::operator>>=(UINT shift) noexcept
{
	*this = (*this) >> shift;
	return *this;
}

/**
 * @brief Perform a not bit operation.
 * 
 * @retval LargeUInt
 * @return The notted LargeUInt.
 */
LargeUInt LargeUInt::operator~() const noexcept
{
	LargeUInt cmp = *this;
	for (int i = 0; i < cmp.size(); i++)
	{
		cmp.m_int[i] = ~cmp.m_int[i];
	}
	return cmp;
}

LargeUInt LargeUInt::createMask(size_t size)
{
	LargeUInt mask = 1;
	mask <<= size;
	mask -= 1;
}
