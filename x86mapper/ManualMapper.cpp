#include "ManualMapper.h"

void* ManualMapper::InjectDll(const std::string &_processName, const std::string &_dllName)
{
	if (!std::experimental::filesystem::exists(_dllName))
	{
		std::cout << "File " << _dllName << " doesn't exist" << std::endl;
		return nullptr;
	}
	
	DWORD processId = gProcess.GetProcessIdByName(_processName);

	if (!processId)
	{
		std::cout << "Process " << _processName << " not found" << std::endl;
		return nullptr;
	}

	HANDLE processHandle = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, false, processId);

	if (!processHandle || processHandle == INVALID_HANDLE_VALUE)
	{
		std::cout << "Error: Failed to obtain handle to the process" << std::endl;
		return nullptr;
	}
	
	byte* rawImage = gFile.ReadToMemory(_dllName);

	if (!rawImage)
	{
		std::cout << "Error: Failed to read " << _dllName << " to memory" << std::endl;
		CloseHandle(processHandle);
		return nullptr;
	}

	void* mappedImage = this->MapImageToProcess(processHandle, rawImage);
	
	gMemory.FreeLocal(rawImage);
	CloseHandle(processHandle);

	return mappedImage;
}

void* ManualMapper::MapImageToProcess(HANDLE _process, byte* _rawImage)
{
	PortableExecutable pe;

	if (!pe.Parse(_rawImage))
		return nullptr;

	void* localImage = gMemory.AllocateLocal(pe.imageSize);
	void* remoteImage = gMemory.AllocateRemote(pe.imageSize, _process);

	this->CopyImageHeaders(localImage, _rawImage, pe.optionalHeader.SizeOfHeaders);
	this->CopyImageSections(localImage, _rawImage, pe.sections);
	this->ResolveRelocsbyDelta(pe.GetRelocs(localImage), (DWORD)remoteImage - pe.optionalHeader.ImageBase);

	if (!this->ResolveImports(_process, pe.GetImports(localImage), localImage))
	{
		gMemory.FreeLocal(localImage);
		gMemory.FreeRemote(remoteImage, _process);
		return nullptr;
	}
	
	WriteProcessMemory(_process, remoteImage, localImage, pe.imageSize, nullptr);
	gMemory.FreeLocal(localImage);

	if (!this->CallRemoteEntryPoint(_process, remoteImage))
	{
		gMemory.FreeRemote(remoteImage, _process);
		return nullptr;
	}
	
	return remoteImage;
}

void ManualMapper::CopyImageHeaders(void* _localImage, byte* _rawImage, DWORD _sizeOfHeaders)
{
	memcpy(_localImage, _rawImage, _sizeOfHeaders);
}

void ManualMapper::CopyImageSections(void* _localImage, byte* _rawImage, vecSections _sections)
{
	for (const auto &section : _sections)
	{
		auto localSection = (void*)((byte*)_localImage + section.VirtualAddress);
		memcpy(localSection, (void*)(_rawImage + section.PointerToRawData), section.SizeOfRawData);
	}
}

void ManualMapper::ResolveRelocsbyDelta(vecRelocs _relocs, DWORD _delta)
{
	for (const auto &reloc : _relocs)
	{
		for (auto i = 0; i < reloc.count; ++i)
		{
			WORD type = reloc.item[i] >> 12;
			WORD offset = reloc.item[i] & 0xFFF;

			if (type == IMAGE_REL_BASED_HIGHLOW)
				*(DWORD*)(reloc.address + offset) += _delta;
		}
	}
}

bool ManualMapper::ResolveImports(HANDLE _process, vecImports _imports, void* _localImage)
{
	for (auto& import : _imports)
	{
		auto importName = (char*)((byte*)_localImage + import->Name);
				
		HMODULE remoteModule = this->ResolveDependency(_process, importName);

		if (!(void*)remoteModule)
		{
			std::cout << "Error: Failed to resolve dependency: " << importName << std::endl;
			return false;
		}

		auto firstThunk = (IMAGE_THUNK_DATA32*)((byte*)_localImage + import->FirstThunk);
		auto originalFirstThunk = (IMAGE_THUNK_DATA32*)((byte*)_localImage + import->OriginalFirstThunk);

		for (; originalFirstThunk->u1.Function; ++originalFirstThunk, ++firstThunk)
		{
			auto thunkData = (IMAGE_IMPORT_BY_NAME*)((byte*)_localImage + originalFirstThunk->u1.AddressOfData);
			firstThunk->u1.Function = (DWORD)gProcess.GetRemoteProcAddress(GetProcessId(_process), LoadLibrary(importName), remoteModule, (LPCSTR)thunkData->Name, importName);
		}
	}

	return true;
}

HMODULE ManualMapper::ResolveDependency(HANDLE _process, const std::string &_moduleName)
{
	std::string moduleName = _moduleName;

	if (!gProcess.GetRemoteModuleHandle(GetProcessId(_process), moduleName)) // dependency wasn't found, if it's a "virtual" module(like APIs) let's get its real name
	{
		std::string modulePath = gFile.GetModuleDirectory(_moduleName);
		moduleName = gFile.GetBaseName(modulePath);
		
		if (!gProcess.GetRemoteModuleHandle(GetProcessId(_process), moduleName)) // nothing is found, map the dependency
		{
			//std::cout << "Loading dependency " << _moduleName << " (" << modulePath << ")" << std::endl;
			
			byte* rawImage = gFile.ReadToMemory(modulePath);
			void* mappedImage = this->MapImageToProcess(_process, rawImage);

			gMemory.FreeLocal(rawImage);
			return (HMODULE)mappedImage;
		}
	}
	
	return gProcess.GetRemoteModuleHandle(GetProcessId(_process), moduleName);
}

bool ManualMapper::CallRemoteEntryPoint(HANDLE _process, void* _remoteImage)
{
	auto stubSize = (SIZE_T)this->LoadDllEnd - (SIZE_T)this->LoadDll;
	
	void* loaderMemory = gMemory.AllocateRemote(stubSize, _process);
	WriteProcessMemory(_process, loaderMemory, this->LoadDll, stubSize, nullptr);

	HANDLE thread = CreateRemoteThread(_process, NULL, NULL, (LPTHREAD_START_ROUTINE)loaderMemory, _remoteImage, NULL, NULL);

	if (!thread)
	{
		gMemory.FreeRemote(loaderMemory, _process);
		return false;
	}

	WaitForSingleObject(thread, INFINITE);

	CloseHandle(thread);
	gMemory.FreeRemote(loaderMemory, _process);

	return true;
}

void WINAPI ManualMapper::LoadDll(void* _remoteImage)
{
	typedef bool(WINAPI *DllMainFunc)(HMODULE, DWORD, PVOID);

	auto dosHeader = (IMAGE_DOS_HEADER*)(byte*)_remoteImage;
	auto ntHeaders = (IMAGE_NT_HEADERS32*)((byte*)_remoteImage + dosHeader->e_lfanew);

	if (ntHeaders->OptionalHeader.AddressOfEntryPoint)
	{
		auto entryPoint = (DllMainFunc)((byte*)_remoteImage + ntHeaders->OptionalHeader.AddressOfEntryPoint);
		entryPoint((HMODULE)(byte*)_remoteImage, DLL_PROCESS_ATTACH, NULL);
	}
}

void WINAPI ManualMapper::LoadDllEnd()
{

}
