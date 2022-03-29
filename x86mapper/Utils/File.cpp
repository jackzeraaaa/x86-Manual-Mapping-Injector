#include "File.h"

File gFile;

byte* File::ReadToMemory(const std::string &_filePath)
{
	if (!std::experimental::filesystem::exists(_filePath))
		return nullptr;

	HANDLE fileHandle = CreateFile(_filePath.c_str(), GENERIC_READ, NULL, NULL, OPEN_EXISTING, NULL, NULL);

	if (fileHandle == INVALID_HANDLE_VALUE)
		return nullptr;

	DWORD fileSize = GetFileSize(fileHandle, NULL);

	if (fileSize == INVALID_FILE_SIZE)
	{
		CloseHandle(fileHandle);
		return nullptr;
	}

	auto buffer = (byte*)gMemory.AllocateLocal(fileSize);

	if (!buffer)
	{
		CloseHandle(fileHandle);
		return nullptr;
	}

	DWORD bytesRead;

	if (!ReadFile(fileHandle, buffer, fileSize, &bytesRead, NULL))
	{
		gMemory.FreeLocal(buffer);
		CloseHandle(fileHandle);
		return nullptr;
	}

	CloseHandle(fileHandle);
	return buffer;
}

std::string File::GetModuleDirectory(const std::string &_moduleName)
{
	char modulePath[MAX_PATH];
	GetModuleFileName(LoadLibrary(_moduleName.c_str()), modulePath, sizeof(modulePath));

	return modulePath;
}

std::string File::GetBaseName(const std::string &_filePath)
{
	return std::experimental::filesystem::path(_filePath).filename().string();
}