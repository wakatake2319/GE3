#include "stringUtility.h"

namespace stringUtility {
	// stringをwstringに変換
	std::wstring ConvertString(const std::string& str) {
		size_t size = str.size();
		std::wstring wstr(size, L' ');
		mbstowcs_s(nullptr, &wstr[0], size + 1, str.c_str(), size);
		return wstr;
	}
	
	// wstringをstringに変換
	std::string ConvertString(const std::wstring& str) {
		size_t size = str.size();
		std::string narrowStr(size, ' ');
		wcstombs_s(nullptr, &narrowStr[0], size + 1, str.c_str(), size);
		return narrowStr;
	}
}; // namespace stringUtility