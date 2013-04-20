#include "Helpers.hpp"
#include <stdexcept>

int HexCharToValue(int c)
{
	switch (c)
	{
	case '0': return 0;
	case '1': return 1;
	case '2': return 2;
	case '3': return 3;
	case '4': return 4;
	case '5': return 5;
	case '6': return 6;
	case '7': return 7;
	case '8': return 8;
	case '9': return 9;
	case 'A': return 10;
	case 'a': return 10;
	case 'B': return 11;
	case 'b': return 11;
	case 'C': return 12;
	case 'c': return 12;
	case 'D': return 13;
	case 'd': return 13;
	case 'E': return 14;
	case 'e': return 14;
	case 'F': return 15;
	case 'f': return 15;
	default: throw std::logic_error("HexCharToValue of nonhex char");
	}
}

std::string splitWideChar(wchar_t c)
{
    std::string out;
    if (c & 0xff000000) out.push_back(c >> 24);
    if (c & 0xff0000) out.push_back(c >> 16);
    if (c & 0xff00) out.push_back(c >> 8);
    out.push_back(c);
    return out;
}
std::string toStr(std::wstring arg)
{
    std::string out;
    for (auto c : arg) out += splitWideChar(c);
    return out;
}

int getTypeOfUtf8Char(int x)
{
    if (x >= 0x010000) return 4;
    if (x >= 0x0800) return 3;
    if (x >= 0x0080) return 2;
    return 1;
}
wchar_t toWideChar(int c)
{
    unsigned out = 0;
    if (c >= 0x010000)
    {
        out += (0xf0 + (c >> 18)) << 24;
        out += (0x80 + ((c >> 12) & 0x3f)) << 16;
        out += (0x80 + ((c >> 6) & 0x3f)) << 8;
        out += (0x80 + (c & 0x3f));
    }
    else if (c >= 0x0800)
    {
        out += (0xe0 + (c >> 12)) << 16;
        out += (0x80 + ((c >> 6) & 0x3f)) << 8;
        out += (0x80 + (c & 0x3f));
    }
    else if (c >= 0x0080)
    {
        out += (0xc0 + (c >> 6)) << 8;
        out += (0x80 + (c & 0x3f));
    }
    else out = c;
    return wchar_t(out);
}

std::vector<std::pair<int, int>> convertToWide(std::vector<std::pair<int, int>> ranges)
{
    std::vector<std::pair<int, int>> out;
    for (auto elem : ranges) 
    {
        if (elem.first > elem.second) continue;
        if (getTypeOfUtf8Char(elem.first) != getTypeOfUtf8Char(elem.second))
        {
            if (elem.second >= 0x010000)
                out.push_back({toWideChar(0x010000), toWideChar(elem.second)});
            else if (elem.second >= 0x0800)
                out.push_back({toWideChar(0x0800), toWideChar(elem.second)});
            else if (elem.second >= 0x0080)
                out.push_back({toWideChar(0x0080), toWideChar(elem.second)});
            if (elem.first <= 0x00ffff)
                out.push_back({toWideChar(elem.first), toWideChar(0x00ffff)});
            else if (elem.first <= 0x07ff)
                out.push_back({toWideChar(elem.first), toWideChar(0x07ff)});
            else if (elem.first <= 0x007f)
                out.push_back({toWideChar(elem.first), toWideChar(0x007f)});
            if ((elem.first <= 0x007f) && (elem.second >= 0x0800))
                out.push_back({toWideChar(0x0080), toWideChar(0x07ff)});
            if ((elem.first <= 0x07ff) && (elem.second >= 0x010000))
                out.push_back({toWideChar(0x0800), toWideChar(0x00ffff)});
        }
        else out.push_back({toWideChar(elem.first), toWideChar(elem.second)});
    }
    return out;
}

