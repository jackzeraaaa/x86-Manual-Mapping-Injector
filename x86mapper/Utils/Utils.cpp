#include "Utils.h"

Utils gUtils;

std::string Utils::StrGetLower(const std::string &_str)
{
	std::string strTemp = _str;
	std::transform(strTemp.begin(), strTemp.end(), strTemp.begin(), std::tolower);
	return strTemp;
}