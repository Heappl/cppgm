#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <string>
#include <vector>
#include <utility>

// given hex digit character c, return its value
int HexCharToValue(int c);
std::string toStr(std::wstring arg);
wchar_t toWideChar(int c);
std::vector<std::pair<int, int>> convertToWide(std::vector<std::pair<int, int>> ranges);

#endif //HELPERS_HPP

