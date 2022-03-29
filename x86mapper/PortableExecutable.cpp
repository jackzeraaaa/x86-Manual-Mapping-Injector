#include "PortableExecutable.h"

bool PortableExecutable::Parse(byte* _rawImage)
{
	this->dosHeader = (IMAGE_DOS_HEADER*)_rawImage; // Get DOS header

	if (this->dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		std::cout << "Error: Invalid DOS signature" << std::endl;
		return false;
	}

	this->ntHeaders = (IMAGE_NT_HEADERS32*)(this->dosHeader->e_lfanew + _rawImage); // Get NT headers 

	if (this->ntHeaders->Signature != IMAGE_NT_SIGNATURE)
	{
		std::cout << "Error: Invalid NT signature" << std::endl;
		return false;
	}

	if (!(this->ntHeaders->FileHeader.Characteristics & IMAGE_FILE_DLL))
	{
		std::cout << "Error: Image is not dll" << std::endl;
		return false;
	}

	if (this->ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
	{
		std::cout << "Error: Image is not 32 bit" << std::endl;
		return false;
	}

	this->optionalHeader = this->ntHeaders->OptionalHeader; // Get optional header

	this->imageSize = this->ntHeaders->OptionalHeader.SizeOfImage; // Get size of image

	auto sectionHeader = IMAGE_FIRST_SECTION(this->ntHeaders); // Get image sections

	for (auto i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i)
		this->sections.emplace_back(sectionHeader[i]);

	return true;
}

vecRelocs PortableExecutable::GetRelocs(void* _imageBase)
{
	vecRelocs relocs;

	auto baseRelocation = this->GetDataDirectory<IMAGE_BASE_RELOCATION*>(IMAGE_DIRECTORY_ENTRY_BASERELOC, _imageBase);

	while (baseRelocation->VirtualAddress)
	{
		RelocInfo relocInfo;

		relocInfo.address = (byte*)_imageBase + baseRelocation->VirtualAddress;
		relocInfo.item = (WORD*)((byte*)baseRelocation + sizeof(IMAGE_BASE_RELOCATION));
		relocInfo.count = (baseRelocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);

		relocs.emplace_back(relocInfo);

		baseRelocation = (IMAGE_BASE_RELOCATION*)((byte*)baseRelocation + baseRelocation->SizeOfBlock);
	}

	return relocs;
}

vecImports PortableExecutable::GetImports(void* _imageBase)
{
	vecImports imports;

	auto import_descriptor = this->GetDataDirectory<IMAGE_IMPORT_DESCRIPTOR*>(IMAGE_DIRECTORY_ENTRY_IMPORT, _imageBase);

	for (; import_descriptor->FirstThunk; ++import_descriptor)
		imports.emplace_back(import_descriptor);

	return imports;
}
