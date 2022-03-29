#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include "Utils.h"
#include "Memory.h"

class Process
{
public:
	DWORD GetProcessIdByName(const std::string &_processName);
	HMODULE GetRemoteModuleHandle(DWORD _processId, const std::string &_moduleName, bool _ignoreUpperCase = true);
	FARPROC GetRemoteProcAddress(DWORD _processId, HMODULE _localModule, HMODULE _remoteModule, LPCSTR _functionName, const std::string &_moduleName);
};

extern Process gProcess;