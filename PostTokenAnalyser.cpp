#include "PostTokenAnalyser.hpp"
#include "StandardData.hpp"
#include "Helpers.hpp"
#include "RegexRule.hpp"
#include "StateMachine.hpp"
#include "Lexer.hpp"
#include <vector>
#include <algorithm>
#include <iostream>

PostTokenAnalyser::PostTokenAnalyser(std::shared_ptr<IPostTokenStream> output)
    : output(output)
{
}

void PostTokenAnalyser::emit_whitespace_sequence()
{
    //ignoring
}

void PostTokenAnalyser::emit_new_line()
{
    //ignoring
}

void PostTokenAnalyser::emit_header_name(const string& data)
{
}

void PostTokenAnalyser::emit_identifier(const string& data)
{
    auto it = StringToTokenTypeMap.find(data);
    if (it != StringToTokenTypeMap.end())
        output->emit_simple(data, it->second);
    else
        output->emit_identifier(data);
}

bool isToBigOctal(const std::string& val)
{
    if (val.size() > 22) return true;
    if (val.size() < 22) return false;
    return (val > "1777777777777777777777");
}

bool isToBigHex(const std::string& val)
{
    return (val.size() > 16);
}

bool isToBigDec(const std::string& val, bool isUnsigned)
{
    if (val.size() > 20) return true;
    if (not isUnsigned && (val.size() > 19)) return true;
    if (val.size() < 19) return false;
    if (not isUnsigned && (val.size() == 19)) return (val >= "9223372036854775808");
    if (val.size() == 20) return (val >= "18446744073709551616");
    return false;
}

template <typename T, typename ValueType, typename Emitter>
void emitIntLiteral(const std::string& data, ValueType value, bool isUnsigned, bool isDec, Emitter output)
{
    if (value > ValueType(std::numeric_limits<T>::max()))
    {
        if (isUnsigned)
            emitIntLiteral<typename NumericProgression<T>::top_unsigned_type, ValueType>(
                data, value, isUnsigned, isDec, output);
        else if (isDec)
            emitIntLiteral<typename NumericProgression<T>::top_dec_type, ValueType>(
                data, value, isUnsigned, isDec, output);
        else
            emitIntLiteral<typename NumericProgression<T>::top_type, ValueType>(
                data, value, isUnsigned, isDec, output);
    }
    //else if (value < std::numeric_limits<T>::min())
        //emitLiteral<typename NumericProgression<T>::bottom_type, ValueType>(data, value, isUnsigned, output);
    else
    {
        T emitted = T(value);
        output->emit_literal(data, FundamentalTypeOf<T>(), &emitted, sizeof(T));
    }
}

template <typename T, typename Emitter>
void emitIntLiteral(const std::string& data, const string& val, bool isHex, bool isOctet, bool isUnsigned, Emitter output)
{
    bool isDec = not isOctet && not isHex;
    if ((isOctet && isToBigOctal(val)) || (isHex && isToBigHex(val)) || (isDec && isToBigDec(val, isUnsigned)))
    {
        output->emit_invalid(data);
        return;
    } 
    else if (val[0] == '-')
    {
        if (isUnsigned) output->emit_invalid(data);
        else emitIntLiteral<T>(data, PA2Decode<long long>(val, isHex, isOctet), false, isDec, output);
    }
    else emitIntLiteral<T, unsigned long long int>(
        data, PA2Decode<unsigned long long>(val, isHex, isOctet), isUnsigned, isDec, output);
}
template <typename T, typename Emitter>
void emitFloatLiteral(const std::string& data, Emitter output)
{
    T aux = PA2Decode<T>(data);
    output->emit_literal(data, FundamentalTypeOf<T>(), &aux, sizeof(aux));
}

template <typename Emitter>
void emitUserDefinedNumber(
    const std::string& data,
    const std::string& prefix,
    const std::string& suffix,
    bool isOctet,
    bool isHex,
    bool isFloat,
    Emitter output)
{
    auto index = suffix.find('_');
    std::string userSuffix = suffix.substr(index);
    std::string value = (isHex ? "0x" : (isOctet ? "0" : "")) + prefix + suffix.substr(0, index);
    if (isFloat) output->emit_user_defined_literal_floating(data, userSuffix, value);
    else output->emit_user_defined_literal_integer(data, userSuffix, value);
}

void PostTokenAnalyser::emit_pp_number(const string& data)
{
    auto auxData = data;
    auto isHex = (auxData.substr(0, 2) == "0x");
    auto isOctet = (not isHex && (auxData[0] == '0') && (auxData.size() > 1)
                    && (auxData[1] >= '0') && (auxData[1] <= '7'));
    if (isHex) auxData = auxData.substr(2);
    if (isOctet) auxData = auxData.substr(1);

    auto digs = "0123456789.";
    if (isHex) digs = "0123456789ABCDEFabcdef";
    if (isOctet) digs = "01234567";
    auto index = auxData.find_first_not_of(digs);
    auto prefix = (index != std::string::npos) ? auxData.substr(0, index) : auxData;
    auto suffix = (index != std::string::npos) ? auxData.substr(index) : "";

    index = prefix.find_first_not_of("0");
    if (index == std::string::npos) index = prefix.size() - 1;
    auto value = prefix.substr(index);
    auto isUnsigned = ((value.size() == 22) && (value[0] == '1'));

    auto longSuffix = chset(L"lL");
    auto longLongSuffix = Rule(L"ll") | L"LL";
    auto unsignedSuffix = chset(L"uU");
    auto unsignedLongSuffix = (chset(L"Uu") >> chset(L"lL")) | (chset(L"lL") >> chset(L"Uu"));
    auto unsignedLongLongSuffix = (chset(L"Uu") >> longLongSuffix) | (longLongSuffix >> chset(L"Uu"));
    auto userDefinedSuffix = L"_" >> +anychar;
    auto eNotation = chset(L"eE") >> ((chset(L"+-") >> +chset(L"0-9")) | +chset(L"0-9"));
    auto floatSuffix = chset(L"fF") | (eNotation >> chset(L"fF"));

    auto dots = std::count(prefix.begin(), prefix.end(), '.');

    if (dots == 0)
    {
        if (suffix.empty())
            emitIntLiteral<int>(data, value, isHex, isOctet, isUnsigned, output);
        else if (matches(longSuffix, suffix))
            emitIntLiteral<long int>(data, value, isHex, isOctet, isUnsigned, output);
        else if (matches(longLongSuffix, suffix))
            emitIntLiteral<long long int>(data, value, isHex, isOctet, isUnsigned, output);
        else if (matches(unsignedSuffix, suffix))
            emitIntLiteral<unsigned int>(data, value, isHex, isOctet, true, output);
        else if (matches(unsignedLongSuffix, suffix))
            emitIntLiteral<unsigned long int>(data, value, isHex, isOctet, true, output);
        else if (matches(unsignedLongLongSuffix, suffix))
            emitIntLiteral<unsigned long long int>(data, value, isHex, isOctet, true, output);
        else if (matches(eNotation, suffix))
            emitFloatLiteral<double>(data, output);
        else if (matches(floatSuffix, suffix))
            emitFloatLiteral<float>(data, output);
        else if (matches(userDefinedSuffix, suffix))
            emitUserDefinedNumber(data, prefix, suffix, isOctet, isHex, false, output);
        else if (matches(eNotation >> userDefinedSuffix, suffix))
            emitUserDefinedNumber(data, prefix, suffix, isOctet, isHex, true, output);
        else output->emit_invalid(data);
    }
    else if ((dots < 2) && (suffix.empty() || matches(eNotation, suffix)))
        emitFloatLiteral<double>(data, output);
    else if ((dots < 2) && matches(floatSuffix, suffix))
        emitFloatLiteral<float>(data, output);
    else if (matches(userDefinedSuffix | (eNotation >> userDefinedSuffix), suffix))
        emitUserDefinedNumber(data, prefix, suffix, isOctet, isHex, true, output);
    else output->emit_invalid(data);
}

unsigned toInt(char c)
{
    return unsigned(c) & 0xff;
}
bool checkIfUtf8(const string& str)
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
unsigned utf8toInt(const string& str)
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

enum class StringCoding
{
    Utf8, Utf16, Utf32
};

std::string toUtf16(uint32_t arg)
{
    if (arg < 0x10000) return std::string((char*)&arg, (char*)&arg + 2);
    arg -= 0x10000;
    uint32_t aux = (0xD800 + ((arg >> 10) & 0x3FF)) + ((0xDC00 + (arg & 0x3FF)) << 16);
    return std::string((char*)&aux, (char*)&aux + 4);
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

void PostTokenAnalyser::emit_character_literal(const string& data)
{
    auto index = data.find('\'');
    auto prefix = data.substr(0, index);
    auto str = convertEscapeSeq(data.substr(index + 1, data.size() - index - 2), StringCoding::Utf8);
    if (not checkIfUtf8(str))
    {
        output->emit_invalid(data);
        return;
    }
    auto repr = utf8toInt(str);
    if (prefix.empty())
    {
        if (str.size() == 1)
            output->emit_literal(data, FundamentalTypeOf<char>(), str.c_str(), sizeof(char));
        else if ((str.size() <= 4) && (str.size() > 0))
            output->emit_literal(data, FundamentalTypeOf<int>(), &repr, sizeof(int));
        else
            output->emit_invalid(data);
    }
    else if ((prefix == "u") && (repr <= 0xffff))
        output->emit_literal(data, FundamentalTypeOf<char16_t>(), &repr, sizeof(char16_t));
    else if (prefix == "U")
        output->emit_literal(data, FundamentalTypeOf<char32_t>(), &repr, sizeof(char32_t));
    else if (prefix == "L")
        output->emit_literal(data, FundamentalTypeOf<wchar_t>(), &repr, sizeof(wchar_t));
    else
        output->emit_invalid(data);
}

template <typename Emitter>
void emit_user_defined_char_array(const string& data, std::string quote, Emitter output)
{
    auto index = data.find_last_of(quote);
    auto value = data.substr(1, index - 1);
    auto suffix = data.substr(index + 1);
    if (suffix.empty() || (suffix[0] != '_'))
        output->emit_invalid(data);
    else if (quote == "'")
        output->emit_user_defined_literal_character(
            data, suffix, FundamentalTypeOf<char>(), value.c_str(), value.size());
    else
        output->emit_user_defined_literal_string_array(
            data, suffix, value.size() + 1, FundamentalTypeOf<char>(), value.c_str(), value.size() + 1);
}

void PostTokenAnalyser::emit_user_defined_character_literal(const string& data)
{
    emit_user_defined_char_array(data, "'", output);
}

template <typename T, typename Emitter>
void emitLiteralArray(const std::string& data, std::vector<char> value, Emitter output)
{
    auto size = value.size();
    for (unsigned i = 0; i < sizeof(T) + 1; ++i) value.push_back('\0');
    output->emit_literal_array(data, size / sizeof(T) + 1, FundamentalTypeOf<T>(), &value.front(), size + sizeof(T));
}

std::string fillCharType(unsigned size, std::string arg)
{
    if (size < arg.size()) return arg.substr(arg.size() - size);
    while (arg.size() < size) arg.push_back('\0');
    return arg;
}

template <typename CharType>
std::vector<char> codeAndOptionallyConvertEscapeSeqs(
    const std::string arg, StringCoding coding, bool doNotConvertEscapeSeqs)
{
    enum Token { Esc, Char, EoS, Utf8Char };
    typedef std::function<void (uint64_t, std::wstring)> Handler;
    typedef std::function<void(uint64_t, std::wstring, Handler)> InitialLinker;
    typedef std::vector<std::pair<Token, Rule>> Definitions;
    typedef TokenizerChain<TokenizerChainLinkType<uint64_t, uint64_t, Token>> Tokenizer;

    uint64_t EndOfString = -1;

    std::vector<char> out;
    auto tokenHandler = [&](Token token, std::wstring val)
        {
            std::string temp(val.begin(), val.end());
            if (token == Esc) temp = convertEscapeSeq(temp, coding);
            else if (token == Utf8Char)
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
            if (token != EoS)
            {
                unsigned sizeOfChar = std::max(temp.size(), sizeof(CharType));
                for (unsigned i = 0; i < sizeOfChar; ++i)
                {
                    if (i < temp.size()) out.push_back(temp[i]);
                    else out.push_back('\0');
                }
            }
        };
    InitialLinker initialLinker = [](uint64_t c, std::wstring text, Handler h) { h(c, text); };

    auto hex = L"\\x" >> +chset(L"0-9a-fA-F");
    auto octDig = chset(L"0-7");
    auto oct = L"\\" >> (octDig | (octDig >> octDig) | (octDig >> octDig >> octDig));
    auto asciiChar = chset(L"\x00-\x7f");
    auto utf8char = chset(L"\xc0-\xff\xffffffc0-\xffffffff") >> +chset(L"\x80-\xbf\xffffff80-\xffffffbf");
    auto escChar = L"\\" >> anychar;
    auto end = chset(EndOfString);

    Definitions defs = {{Utf8Char, utf8char}, {Char, anychar}, {EoS, end}};
    if (not doNotConvertEscapeSeqs) defs.push_back({Esc, hex | oct | escChar});
    auto tokenizer = Tokenizer(tokenHandler, std::pair<InitialLinker, Definitions>(initialLinker, defs));
    for (auto c : arg) tokenizer.handler(wcharToUint64(c), {wchar_t(c)});
    tokenizer.handler(EndOfString, {wchar_t(EndOfString)});
    return out;
}

void PostTokenAnalyser::emit_string_literal(const string& data)
{
    auto index = data.find('"');
    std::string prefix = data.substr(0, index);
    std::string value = data.substr(index + 1, data.size() - index - 2);
    bool isRaw = false;
    if (not prefix.empty() && (prefix[prefix.size() - 1] == 'R'))
    {
        prefix = prefix.substr(0, prefix.size() - 1);
        index = value.find('(');
        value = value.substr(index + 1, value.size() - 2 * index - 2); 
        isRaw = true;
    }
    if ((prefix == "u8") || (prefix.empty()))
        emitLiteralArray<char>(
            data, codeAndOptionallyConvertEscapeSeqs<char>(value, StringCoding::Utf8, isRaw), output);
    else if (prefix == "u")
        emitLiteralArray<char16_t>(
            data, codeAndOptionallyConvertEscapeSeqs<char16_t>(value, StringCoding::Utf16, isRaw), output);
    else if (prefix == "U")
        emitLiteralArray<char32_t>(
            data, codeAndOptionallyConvertEscapeSeqs<char32_t>(value, StringCoding::Utf32, isRaw), output);
    else if (prefix == "L")
        emitLiteralArray<wchar_t>(
            data, codeAndOptionallyConvertEscapeSeqs<wchar_t>(value, StringCoding::Utf32, isRaw), output);
}

void PostTokenAnalyser::emit_user_defined_string_literal(const string& data)
{
    emit_user_defined_char_array(data, "\"", output);
}

void PostTokenAnalyser::emit_preprocessing_op_or_punc(const string& data)
{
    auto it = StringToTokenTypeMap.find(data);
    if (it != StringToTokenTypeMap.end())
        output->emit_simple(data, it->second);
    else
        output->emit_invalid(data);
}

void PostTokenAnalyser::emit_non_whitespace_char(const string& data)
{
    output->emit_invalid(data);
}

void PostTokenAnalyser::emit_eof()
{
    output->emit_eof();
}

