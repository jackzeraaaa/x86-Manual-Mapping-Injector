#include "Process.h"

Process gProcess;

DWORD Process::GetProcessIdByName(const std::string &_processName)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (snapshot == INVALID_HANDLE_VALUE)
		return 0;

	PROCESSENTRY32 processEntry;
	processEntry.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(snapshot, &processEntry))
	{
		CloseHandle(snapshot);
		return 0;
	}

	do
	{
		if (!_processName.compare(processEntry.szExeFile))
		{
			CloseHandle(snapshot);
			return processEntry.th32ProcessID;
		}
	} while (Process32Next(snapshot, &processEntry));

	CloseHandle(snapshot);
	return 0;
}

HMODULE Process::GetRemoteModuleHandle(DWORD _processId, const std::string &_moduleName, bool _ignoreUpperCase)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, _processId);

	if (snapshot == INVALID_HANDLE_VALUE)
		return NULL;

	MODULEENTRY32 moduleEntry;
	moduleEntry.dwSize = sizeof(MODULEENTRY32);

	if (!Module32First(snapshot, &moduleEntry))
	{
		CloseHandle(snapshot);
		return NULL;
	}

	do
	{
		if ((_ignoreUpperCase)
			? !gUtils.StrGetLower(moduleEntry.szModule).compare(gUtils.StrGetLower(_moduleName))
			: !_moduleName.compare(moduleEntry.szModule))
		{
			CloseHandle(snapshot);
			return moduleEntry.hModule;
		}
	} while (Module32Next(snapshot, &moduleEntry));

	CloseHandle(snapshot);
	return NULL;
}

FARPROC Process::GetRemoteProcAddress(DWORD _processId, HMODULE _localModule, HMODULE _remoteModule, LPCSTR _functionName, const std::string &_moduleName)
{
	DWORD delta = (DWORD)_remoteModule - (DWORD)_localModule;
	return (FARPROC)((DWORD)GetProcAddress(_localModule, _functionName) + delta);
}