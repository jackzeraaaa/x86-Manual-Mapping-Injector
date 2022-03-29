#pragma once
#include <Windows.h>

class Memory
{
public:
	LPVOID AllocateLocal(SIZE_T _size, LPVOID _address = nullptr);
	BOOL FreeLocal(LPVOID _address, SIZE_T _size = 0);
	LPVOID AllocateRemote(SIZE_T _size, HANDLE _processHandle, LPVOID _address = nullptr);
	BOOL FreeRemote(LPVOID _address, HANDLE _processHandle, SIZE_T _size = 0);
};

extern Memory gMemory;