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

char ValueToHexChar(int c)
{
	switch (c)
	{
	case 0: return '0';
	case 1: return '1';
	case 2: return '2';
	case 3: return '3';
	case 4: return '4';
	case 5: return '5';
	case 6: return '6';
	case 7: return '7';
	case 8: return '8';
	case 9: return '9';
	case 10: return 'A';
	case 11: return 'B';
	case 12: return 'C';
	case 13: return 'D';
	case 14: return 'E';
	case 15: return 'F';
	default: throw std::logic_error("ValueToHexChar of nonhex value");
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

std::string toStr(int arg)
{
    std::string ret;
    ret.push_back(char(arg));
    return ret;
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
wchar_t toWideCharInUtf8(int c)
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
                out.push_back({toWideCharInUtf8(0x010000), toWideCharInUtf8(elem.second)});
            else if (elem.second >= 0x0800)
                out.push_back({toWideCharInUtf8(0x0800), toWideCharInUtf8(elem.second)});
            else if (elem.second >= 0x0080)
                out.push_back({toWideCharInUtf8(0x0080), toWideCharInUtf8(elem.second)});
            if (elem.first <= 0x00ffff)
                out.push_back({toWideCharInUtf8(elem.first), toWideCharInUtf8(0x00ffff)});
            else if (elem.first <= 0x07ff)
                out.push_back({toWideCharInUtf8(elem.first), toWideCharInUtf8(0x07ff)});
            else if (elem.first <= 0x007f)
                out.push_back({toWideCharInUtf8(elem.first), toWideCharInUtf8(0x007f)});
            if ((elem.first <= 0x007f) && (elem.second >= 0x0800))
                out.push_back({toWideCharInUtf8(0x0080), toWideCharInUtf8(0x07ff)});
            if ((elem.first <= 0x07ff) && (elem.second >= 0x010000))
                out.push_back({toWideCharInUtf8(0x0800), toWideCharInUtf8(0x00ffff)});
        }
        else out.push_back({toWideCharInUtf8(elem.first), toWideCharInUtf8(elem.second)});
    }
    return out;
}

std::string HexDump(const void* pdata, size_t nbytes)
{
	unsigned char* p = (unsigned char*) pdata;

	std::string s(nbytes*2, '?');

	for (size_t i = 0; i < nbytes; i++)
	{
		s[2*i+0] = ValueToHexChar((p[i] & 0xF0) >> 4);
		s[2*i+1] = ValueToHexChar((p[i] & 0x0F) >> 0);
	}

	return s;
}

uint64_t wcharToUint64(wchar_t arg)
{
    return uint64_t(arg) & uint64_t(0xffffffff);
}

