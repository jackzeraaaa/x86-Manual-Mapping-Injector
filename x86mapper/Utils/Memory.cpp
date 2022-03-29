#include "Memory.h"

Memory gMemory;

LPVOID Memory::AllocateLocal(SIZE_T _size, LPVOID _address)
{
	return VirtualAlloc(_address, _size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}

BOOL Memory::FreeRemote(LPVOID _address, HANDLE _processHandle, SIZE_T _size)
{
	return VirtualFreeEx(_processHandle, _address, _size, MEM_RELEASE);
}

LPVOID Memory::AllocateRemote(SIZE_T _size, HANDLE _processHandle, LPVOID _address)
{
	return VirtualAllocEx(_processHandle, _address, _size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}

BOOL Memory::FreeLocal(LPVOID _address, SIZE_T _size)
{
	return VirtualFree(_address, _size, MEM_RELEASE);
}