#pragma once

#include <string>
#include <vector>
#include <utility>
#include <sstream>

// given hex digit character c, return its value
int HexCharToValue(int c);
// convert integer [0,15] to hexadecimal digit
char ValueToHexChar(int c);

std::string splitWideChar(wchar_t c);
std::string toStr(int arg);
std::string toStr(std::wstring arg);
wchar_t toWideCharInUtf8(int c);
std::vector<std::pair<int, int>> convertToWide(std::vector<std::pair<int, int>> ranges);

// hex dump memory range
std::string HexDump(const void* pdata, size_t nbytes);

// use these 3 functions to scan `floating-literals` (see PA2)
// for example PA2Decode_float("12.34") returns "12.34" as a `float` type
template <typename T>
T PA2Decode(const std::string& s, bool isHex = false, bool isOctet = false)
{
	std::stringstream ss;
    if (isHex) ss << std::hex;
    if (isOctet) ss << std::oct;
    ss << s;
	T x;
	ss >> x;
	return x;
}

uint64_t wcharToUint64(wchar_t arg);

