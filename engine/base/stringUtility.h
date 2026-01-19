#pragma once
#include <string>

// 文字コードセキュリティ
namespace stringUtility {
	// stringをwstringに変換
	std::wstring ConvertString(const std::string& str);
	
	// wstringをstringに変換
	std::string ConvertString(const std::wstring& str);
};
