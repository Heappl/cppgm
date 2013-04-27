#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <string>
#include <vector>
#include <utility>

// given hex digit character c, return its value
int HexCharToValue(int c);
// convert integer [0,15] to hexadecimal digit
char ValueToHexChar(int c);

std::string toStr(std::wstring arg);
wchar_t toWideChar(int c);
std::vector<std::pair<int, int>> convertToWide(std::vector<std::pair<int, int>> ranges);

// hex dump memory range
std::string HexDump(const void* pdata, size_t nbytes);

// use these 3 functions to scan `floating-literals` (see PA2)
// for example PA2Decode_float("12.34") returns "12.34" as a `float` type
float PA2Decode_float(const std::string& s);
double PA2Decode_double(const std::string& s);
long double PA2Decode_long_double(const std::string& s);
long long PA2Decode_long_long(const std::string& s, bool isHex,bool isOctet);

#endif //HELPERS_HPP

