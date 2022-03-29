#pragma once
#include "Utils\Utils.h"
#include "Utils\File.h"
#include "Utils\Memory.h"
#include "Utils\Process.h"
#include "PortableExecutable.h"
#include <filesystem>

class ManualMapper
{
public:
	void* InjectDll(const std::string &_processName, const std::string &_dllName);
private:
	void* MapImageToProcess(HANDLE _process, byte* _rawImage);
	void CopyImageHeaders(void* _localImage, byte* _rawImage, DWORD _sizeOfHeaders);
	void CopyImageSections(void* _localImage, byte* _rawImage, vecSections _sections);
	void ResolveRelocsbyDelta(vecRelocs _relocs, DWORD _delta);
	bool ResolveImports(HANDLE _process, vecImports _imports, void* _localImage);
	HMODULE ResolveDependency(HANDLE _process, const std::string &_moduleName);
	bool CallRemoteEntryPoint(HANDLE _process, void* _remoteImage);
private:
	static void WINAPI LoadDll(void* _remoteImage);
	static void WINAPI LoadDllEnd();
};