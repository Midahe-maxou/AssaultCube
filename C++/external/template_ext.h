#pragma once
#ifndef RE_TEMPLATE_EXT
#define RE_TEMPLATE_EXT

#include "template.h"

#ifdef RE_EXT

#include <Windows.h>
#include <vector>
#include <memory> // std::shared_ptr


//uintptr_t* findDMAAddy(_In_ HANDLE hProc, _In_ uintptr_t* basePtr, _In_ const std::vector<UINT>& offsets);


/**
* Dynamic variables are variables that are accessed throught MemoryRead and MemoryWrite.
* Getter and setter can be defined within the header file.
*/

#define CREATE_DYNAMIC_GETTER(type, name, offset)															\
			type get##name##()																				\
			{																								\
				return ReadMemory<type>(addressPtr, offset);												\
			}

#define CREATE_DYNAMIC_SETTER(type, name, offset)															\
			void set##name##(type val)																		\
			{																								\
				WriteMemory<type>(addressPtr, offset, val);													\
			}

#define DEFINE_DYNAMIC_VARIABLE(type, name, offset)															\
			CREATE_DYNAMIC_GETTER(type, name, offset)														\
			CREATE_DYNAMIC_SETTER(type, name, offset)


/**
* Class instance variables are instances to other class.
* Warning: Does not work with circular dependencies
*/

#define CREATE_CLASS_INSTANCE_GETTER(type, name, offset)													\
			std::shared_ptr<type> get##name##()																\
			{																								\
				uintptr_t address = ReadMemory<uintptr_t>(addressPtr, offset);								\
				return std::make_shared<type>(type(address));												\
			}

#define CREATE_CLASS_INSTANCE_SETTER(type, name, offset)													\
			void set##name##(std::shared_ptr<Pointer> val)													\
			{																								\
				WriteMemory<type>(addressPtr, offset, val->getAddressPtr());								\
			}

#define DEFINE_CLASS_INSTANCE_VARIABLE(type, name, offset)													\
			CREATE_CLASS_INSTANCE_GETTER(type, name, offset)												\
			CREATE_CLASS_INSTANCE_SETTER(type, name, offset)

/**
* Same as class instances variables but with circular dependencies protection
* Must be declared after all the class are defined (can be put in the cpp file)
*/

#define DEFINE_CIRCULAR_DEPENDANT_CLASS_INSTANCE_VARIABLE(type, name, offset)								\
			std::shared_ptr<type> get##name##();															\
			void set##name##(std::shared_ptr<Pointer> val);

#define CREATE_CIRCULAR_DEPENDANT_CLASS_INSTANCE_GETTER(classname, type, name, offset)						\
			std::shared_ptr<type> classname::get##name##()													\
			{																								\
				uintptr_t address = ReadMemory<uintptr_t>(addressPtr, offset);								\
				return std::make_shared<type>(type(address));												\
			}

#define CREATE_CIRCULAR_DEPENDANT_CLASS_INSTANCE_SETTER(classname, type, name, offset)						\
			void classname::set##name##(std::shared_ptr<Pointer> val)										\
			{																								\
				WriteMemory<intptr_t>(addressPtr, offset, val->getAddressPtr());							\
			}

#define CREATE_CIRCULAR_DEPENDANT_CLASS_INSTANCE_VARIABLE_METHODS(classname, type, name, offset)			\
			CREATE_CIRCULAR_DEPENDANT_CLASS_INSTANCE_GETTER(classname, type, name, offset)					\
			CREATE_CIRCULAR_DEPENDANT_CLASS_INSTANCE_SETTER(classname, type, name, offset)


/**
* Pointer variables are pointers to primitive types.
*/

#define CREATE_POINTER_GETTER(type, name, offset)															\
		type get##name##()																					\
		{																									\
			uintptr_t address = ReadMemory<uintptr_t>(addressPtr, offset);									\
			return ReadMemory<type>(address, 0);															\
		}

#define CREATE_POINTER_SETTER(type, name, offset)															\
		void set##name##(type val)																			\
		{																									\
			uintptr_t address = ReadMemory<uintptr_t>(addressPtr, offset);									\
			WriteMemory<type>(address, 0, val);																\
		}

#define DEFINE_POINTER_VARIABLE(type, name, offset)															\
		CREATE_POINTER_GETTER(type, name, offset)															\
		CREATE_POINTER_SETTER(type, name, offset)															\

/**
* Multi level pointer variables are multi level pointers to primitive types
*/

#define CREATE_MULTI_LEVEL_POINTER_GETTER(type, name, ...)													\
		type get##name##()																					\
		{																									\
			OFFSET_LIST offsetList{ __VA_ARGS__ };															\
			return ReadMemoryWithOffsetList<type>(addressPtr, offsetList);									\
		}

#define CREATE_MULTI_LEVEL_POINTER_SETTER(type, name, ...)													\
		void set##name##(type val)																			\
		{																									\
			OFFSET_LIST offsetList{ __VA_ARGS__ };															\
			WriteMemoryWithOffsetList<type>(addressPtr, offsetList, val);									\
		}

#define DEFINE_MULTI_LEVEL_POINTER_VARIABLE(type, name, ...)												\
		CREATE_MULTI_LEVEL_POINTER_GETTER(type, name, __VA_ARGS__)											\
		CREATE_MULTI_LEVEL_POINTER_SETTER(type, name, __VA_ARGS__)											\



/**
* Read-Only variables are flags. They are like dynamic variables, but cannot be overwritten.
*/

#define CREATE_RO_GETTER(type, name, offset)													\
			type name()																			\
			{																					\
				return ReadMemory<type>(addressPtr, offset);									\
			}

#define DEFINE_RO_VARIABLE(type, name, offset)													\
			CREATE_RO_GETTER(type, name, offset)
			

// Global variable
static HANDLE hProc = nullptr;


int init(_In_ int procId);
HANDLE gethProc();


const uintptr_t getAddressWithOffsetList(_In_ uintptr_t address, _In_opt_ OFFSET_LIST offsetList);

uintptr_t AllocateAndInjectCode(_In_ const std::vector<BYTE>& code, _In_ uintptr_t injectionAddress, _In_ BYTE opcodeLenght);
bool RetreiveAndDesallocateCode(_In_ uintptr_t injectedAddress, _In_ BYTE opcodeLenght);
std::vector<BYTE> ReadCode(_In_ uintptr_t address, _In_ uintptr_t offset);
std::vector<BYTE> ReadCode(_In_ uintptr_t address, _In_ uintptr_t offset, _In_ size_t size);
bool WriteCode(_In_ uintptr_t address, _In_ uintptr_t offset, _In_ std::vector<BYTE> code);


std::vector<BYTE> revAddressToBytes(_In_ uintptr_t address);
uintptr_t revBytesToAddress(_In_ std::vector<BYTE> bytes);

void concat(_In_ std::vector<BYTE>& vec1, _In_ const std::vector<BYTE>& vec2);
std::vector<BYTE> getNops(_In_ int numOfNop);

// Template functions

template <class T>
T ReadMemory(_In_ uintptr_t address, _In_ OFFSET offset)
{
	T dest;
	ReadProcessMemory(gethProc(), reinterpret_cast<const void*>(address + offset), &dest, sizeof(dest), NULL);
	return dest;
}

template <class T>
bool WriteMemory(_In_ uintptr_t address, _In_ OFFSET offset, _In_ T val)
{
	return WriteProcessMemory(gethProc(), reinterpret_cast<void*>(address + offset), &val, sizeof(val), NULL);
}

template<class T>
T ReadMemoryWithOffsetList(_In_ uintptr_t address, _In_ const OFFSET_LIST& offsetList)
{
	size_t nbOffset = offsetList.size();

	if (nbOffset == 0) return T(address);
	if (nbOffset == 1) return ReadMemory<T>(address, offsetList[0]);

	for (unsigned int i = 0; i < nbOffset - 1; i++)
		address = ReadMemory<uintptr_t>(address, offsetList[i]);

	return ReadMemory<T>(address, offsetList[nbOffset - 1]);
}

template<class T>
void WriteMemoryWithOffsetList(_In_ uintptr_t address, _In_ OFFSET_LIST offsetList, _In_ T val)
{
	OFFSET lastOffset = 0;
	if (offsetList.size() != 0) {
		lastOffset = offsetList.back();
		offsetList.pop_back();
		address = ReadMemoryWithOffsetList<uintptr_t>(address, offsetList);
	}

	WriteMemory<T>(address, lastOffset, val);
}


#endif // RE_EXT

#endif // RE_TEMPLATE_EXT