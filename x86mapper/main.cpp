#include "ManualMapper.h"
#include <iostream>

int main()
{
	ManualMapper mapper;
	//comments
	char dllName[260];
	std::cout << "->  Type dll name without \".dll\": ";
	std::cin >> dllName;
	strcat_s(dllName, ".dll");
	if(!mapper.InjectDll("csgo.exe", dllName))
		getchar();
}