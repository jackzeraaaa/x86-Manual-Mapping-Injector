#pragma once
#include <Windows.h>
#include <fstream>
#include <string>
#include <experimental\filesystem>
#include "Memory.h"

class File
{
public:
	byte* ReadToMemory(const std::string &_filePath);
	std::string GetModuleDirectory(const std::string &_moduleName);
	std::string GetBaseName(const std::string &_filePath);
};

extern File gFile;