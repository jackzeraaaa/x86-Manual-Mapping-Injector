#pragma once
#include <Windows.h>
#include <iostream>
#include <vector>

struct RelocInfo
{
	byte* address;
	WORD* item;
	int count;
};

typedef std::vector<IMAGE_SECTION_HEADER> vecSections;
typedef std::vector<RelocInfo> vecRelocs;
typedef std::vector<IMAGE_IMPORT_DESCRIPTOR*> vecImports;

class PortableExecutable
{
public:
	bool Parse(byte* _rawImage);
	vecRelocs GetRelocs(void* _imageBase);
	vecImports GetImports(void* _imageBase);
public:
	IMAGE_DOS_HEADER* dosHeader;
	IMAGE_NT_HEADERS32* ntHeaders;
	IMAGE_OPTIONAL_HEADER32 optionalHeader;
	vecSections sections;
	DWORD imageSize;
private:
	template <typename IMAGE_DIRECTORY_TYPE>
	auto GetDataDirectory(DWORD _IMAGE_DIRECTORY_ENTRY, void* _imageBase)
	{
		return (IMAGE_DIRECTORY_TYPE)((byte*)_imageBase + this->optionalHeader.DataDirectory[_IMAGE_DIRECTORY_ENTRY].VirtualAddress);
	}
};