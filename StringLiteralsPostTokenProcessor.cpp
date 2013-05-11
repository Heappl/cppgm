#include "StringLiteralsPostTokenProcessor.hpp"
#include "Lexer.hpp"
#include "RegexRule.hpp"
#include "Helpers.hpp"
#include "StandardData.hpp"

#include <algorithm>
#include <cassert>

namespace
{

unsigned toInt(char c)
{
    return unsigned(c) & 0xff;
}

std::string toUtf16(uint32_t arg)
{
    if (arg < 0x10000) return std::string((char*)&arg, (char*)&arg + 2);
    arg -= 0x10000;
    uint32_t aux = (0xD800 + ((arg >> 10) & 0x3FF)) + ((0xDC00 + (arg & 0x3FF)) << 16);
    return std::string((char*)&aux, (char*)&aux + 4);
}

enum class Token { Esc, Char, EoS, Utf8Char };
typedef std::function<void (uint64_t, std::wstring)> Handler;
typedef std::function<void(uint64_t, std::wstring, Handler)> InitialLinker;
typedef std::vector<std::pair<Token, Rule>> Definitions;
typedef TokenizerChain<TokenizerChainLinkType<uint64_t, uint64_t, Token>> Tokenizer;
const uint64_t EndOfString = -1;

template <typename TokenHandler>
void parseString(const std::string& arg, TokenHandler tokenHandler, Definitions defs)
{
    auto asciiChar = chset(L"\x00-\x7f");
    auto end = chset(EndOfString);
    defs.push_back({Token::Char, anychar});
    defs.push_back({Token::EoS, end});

    InitialLinker initialLinker = [](uint64_t c, std::wstring text, Handler h) { h(c, text); };
    auto tokenizer = Tokenizer(tokenHandler, std::pair<InitialLinker, Definitions>({initialLinker, defs}));
    for (auto c : arg) tokenizer.handler(wcharToUint64(c), {wchar_t(c)});
    tokenizer.handler(EndOfString, {wchar_t(EndOfString)});
}

std::string convertEscapeSeqs(const std::string arg)
{
    std::string out = "";
    auto tokenHandler = [&](Token token, std::wstring val)
        {
            std::string temp(val.begin(), val.end());
            if (token == Token::Esc) temp = convertEscapeSeq(temp, StringCoding::Utf8);
            if (token != Token::EoS) out += temp;
        };

    auto hex = L"\\x" >> +chset(L"0-9a-fA-F");
    auto octDig = chset(L"0-7");
    auto oct = L"\\" >> (octDig | (octDig >> octDig) | (octDig >> octDig >> octDig));
    auto escChar = L"\\" >> anychar;

    parseString(arg, tokenHandler, {{Token::Esc, hex | oct | escChar}});
    return out;
}

std::size_t getSizeOfCharFromEncoding(StringCoding coding)
{
    if (coding == StringCoding::Utf8) return sizeof(char);
    if (coding == StringCoding::Utf16) return sizeof(char16_t);
    if (coding == StringCoding::Utf32) return sizeof(char32_t);
    assert(!"invalid coding");
}

std::vector<char> encodeString(
    const std::string arg, StringCoding coding)
{
    std::vector<char> out;
    auto tokenHandler = [&](Token token, std::wstring val)
        {
            std::string temp(val.begin(), val.end());
            if (token == Token::Utf8Char)
            {
                if (coding == StringCoding::Utf16)
                {
                    auto converted = utf8toInt(temp);
                    temp = toUtf16(converted);
                }
                else if (coding == StringCoding::Utf32)
                {
                    auto converted = utf8toInt(temp);
                    temp = std::string((char*)&converted, (char*)&converted + sizeof(converted));
                }
            }
            if (token != Token::EoS)
            {
                unsigned sizeOfChar = std::max(temp.size(), getSizeOfCharFromEncoding(coding));
                for (unsigned i = 0; i < sizeOfChar; ++i)
                {
                    if (i < temp.size()) out.push_back(temp[i]);
                    else out.push_back('\0');
                }
            }
        };

    auto utf8char = chset(L"\xc0-\xff\xffffffc0-\xffffffff") >> +chset(L"\x80-\xbf\xffffff80-\xffffffbf");
    parseString(arg, tokenHandler, {{Token::Utf8Char, utf8char}});
    return out;
}

EFundamentalType max(EFundamentalType first, EFundamentalType second)
{
    std::vector<EFundamentalType> aux = {
        FundamentalTypeOf<char>(), FundamentalTypeOf<char16_t>(),
        FundamentalTypeOf<char32_t>(), FundamentalTypeOf<wchar_t>()};
    unsigned firstIndex = std::find(aux.begin(), aux.end(), first) - aux.begin();
    unsigned secondIndex = std::find(aux.begin(), aux.end(), second) - aux.begin();
    if (firstIndex < secondIndex) return second;
    return first;
}

unsigned getSizeOfElementOfType(EFundamentalType type)
{
    if (type == FundamentalTypeOf<char>()) return sizeof(char);
    if (type == FundamentalTypeOf<char16_t>()) return sizeof(char16_t);
    if (type == FundamentalTypeOf<char32_t>()) return sizeof(char32_t);
    if (type == FundamentalTypeOf<wchar_t>()) return sizeof(wchar_t);
    assert(!"invalid type");
}

StringCoding getCodingFromType(EFundamentalType type)
{
    if (type == FundamentalTypeOf<char>()) return StringCoding::Utf8;
    if (type == FundamentalTypeOf<char16_t>()) return StringCoding::Utf16;
    if (type == FundamentalTypeOf<char32_t>()) return StringCoding::Utf32;
    if (type == FundamentalTypeOf<wchar_t>()) return StringCoding::Utf32;
    assert(!"invalid type");
}

} //namespace

StringLiteralsPostTokenProcessor::StringLiteralsPostTokenProcessor(std::shared_ptr<IPostTokenStream> output)
    : output(output), type(FundamentalTypeOf<void>()), isInvalid(false)
{
}

void StringLiteralsPostTokenProcessor::addString(const std::string& data)
{
    source += (source.empty() ? "" : " ") + data;
    auto index = data.find('"');
    std::string prefix = data.substr(0, index);
    std::string value = data.substr(index + 1, data.size() - index - 2);
    if (not prefix.empty() && (prefix[prefix.size() - 1] == 'R'))
    {
        prefix = prefix.substr(0, prefix.size() - 1);
        index = value.find('(');
        strings.push_back(value.substr(index + 1, value.size() - 2 * index - 2)); 
    }
    else strings.push_back(convertEscapeSeqs(value));
    EFundamentalType auxType = FundamentalTypeOf<void>();
    if (prefix == "u8") auxType = FundamentalTypeOf<char>();
    else if (prefix == "u") auxType = FundamentalTypeOf<char16_t>();
    else if (prefix == "U") auxType = FundamentalTypeOf<char32_t>();
    else if (prefix == "L") auxType = FundamentalTypeOf<wchar_t>();

    if ((auxType != FundamentalTypeOf<void>())
        && (type != FundamentalTypeOf<void>())
        && (type != auxType))
        isInvalid = true;
    if (type == FundamentalTypeOf<void>()) type = auxType;
}

void StringLiteralsPostTokenProcessor::addUserDefinedString(const std::string& data)
{
    auto index = data.rfind('"');
    addString(data.substr(0, index + 1));
    auto suffix = data.substr(index + 1);
    source += suffix;
    if (suffix.empty() || (suffix[0] != '_') || (not userDefinedSuffix.empty() && (userDefinedSuffix != suffix)))
        isInvalid = true;
    else
        userDefinedSuffix = suffix;
}

void StringLiteralsPostTokenProcessor::flush()
{
    if (strings.empty()) return;
    if (not isInvalid)
    {
        if (type == FundamentalTypeOf<void>()) type = FundamentalTypeOf<char>();
        std::vector<char> data;
        for (auto elem : strings)
        {
            auto encoded = encodeString(elem, getCodingFromType(type));
            data.insert(data.end(), encoded.begin(), encoded.end());
        }
        unsigned size = getSizeOfElementOfType(type);
        unsigned elements = data.size() / size + 1;
        for (unsigned i = 0; i < size; ++i) data.push_back('\0');
        if (userDefinedSuffix.empty())
            output->emit_literal_array(source, elements, type, &data.front(), data.size());
        else
            output->emit_user_defined_literal_string_array(
                source, userDefinedSuffix, elements, type, &data.front(), data.size());
    }
    else
    {
        output->emit_invalid(source);
    }
    clear();
}

void StringLiteralsPostTokenProcessor::clear()
{
    strings.clear();
    userDefinedSuffix.clear();
    type = FundamentalTypeOf<void>();
    source.clear();
    isInvalid = false;
}

std::string convertEscapeSeq(std::string arg, StringCoding coding)
{
    if (arg.empty()) return "";
    if (arg[0] != '\\') return arg;
    if (arg.size() == 1) return "";
    if (arg.size() == 2)
    {
        if (arg[1] == 'a') return toStr(0x07);
        if (arg[1] == 'b') return toStr(0x08);
        if (arg[1] == 'f') return toStr(0x0c);
        if (arg[1] == 'n') return toStr(0x0a);
        if (arg[1] == 'r') return toStr(0x0d);
        if (arg[1] == 't') return toStr(0x09);
        if (arg[1] == 'v') return toStr(0x0b);
        if ((arg[1] == '\'') || (arg[1] == '\"') || (arg[1] == '\?') || (arg[1] == '\\'))
            return arg.substr(1);
        if ((arg[1] >= '0') && (arg[1] <= '7')) return toStr(int(arg[1] - '0'));
        return "";
    }
    else if ((arg[1] == 'x') && (arg.size() < 10) && (matches(+chset(L"0-9a-fA-F"), arg.substr(2))))
    {
        uint32_t aux = 0;
        for (auto c : arg.substr(2)) aux = (aux << 4) + HexCharToValue(c);
        if (coding == StringCoding::Utf8) return splitWideChar(toWideCharInUtf8(aux));
        else if (coding == StringCoding::Utf16) return toUtf16(aux);
        else if (coding == StringCoding::Utf32) return std::string((char*)&aux, (char*)&aux + sizeof(aux));
    }
    else if ((arg.size() < 17) && matches(+chset(L"0-7"), arg.substr(1)))
    {
        uint32_t aux = 0;
        for (auto c : arg.substr(1)) aux = (aux << 3) + int(c - '0');
        if (coding == StringCoding::Utf8) return splitWideChar(toWideCharInUtf8(aux));
        else if (coding == StringCoding::Utf16) return toUtf16(aux);
        else if (coding == StringCoding::Utf32) return std::string((char*)&aux, (char*)&aux + sizeof(aux));
    }
    return "";
}
bool checkIfUtf8(const std::string& str)
{
    for (unsigned i = 1; i < str.size(); ++i)
        if ((str[i] & 0xc0) != 0x80) return false;
    if (str.size() == 4) return (str[0] & 0xf0) == 0xf0;
    if (str.size() == 3) return (str[0] & 0xf0) == 0xe0;
    if (str.size() == 2) return (str[0] & 0xf0) == 0xc0;
    if (str.size() == 2) return (str[0] & 0xf0) == 0xc0;
    if (str.size() == 1) return str[0] < 128;
    return false;
}
unsigned utf8toInt(const std::string& str)
{
    if (str.size() == 4)
    {
        return
            ((toInt(str[0]) - 0xf0) << 18)
            + ((toInt(str[1]) - 0x80) << 12)
            + ((toInt(str[2]) - 0x80) << 6)
            + ((toInt(str[3]) - 0x80) << 0);
    }
    if (str.size() == 3)
    {
        return
            ((toInt(str[0]) - 0xe0) << 12)
            + ((toInt(str[1]) - 0x80) << 6)
            + ((toInt(str[2]) - 0x80) << 0);
    }
    if (str.size() == 2)
    {
        return
            ((toInt(str[0]) - 0xc0) << 6)
            + ((toInt(str[1]) - 0x80) << 0);
    }
    return toInt(str[0]);
}

