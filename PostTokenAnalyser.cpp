#include "PostTokenAnalyser.hpp"
#include "StandardData.hpp"
#include "Helpers.hpp"
#include "RegexRule.hpp"
#include "StateMachine.hpp"
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
    return false;
}

bool isToBigDec(const std::string& val)
{
    return false;
}

template <typename T, typename ValueType, typename Emitter>
void emitLiteral(const std::string& data, ValueType value, bool isUnsigned, Emitter output)
{
    if (value > ValueType(std::numeric_limits<T>::max()))
    {
        if (isUnsigned)
            emitLiteral<typename NumericProgression<T>::top_unsigned_type, ValueType>(data, value, isUnsigned, output);
        else
            emitLiteral<typename NumericProgression<T>::top_type, ValueType>(data, value, isUnsigned, output);
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
void emitLiteral(const std::string& data, const string& val, bool isHex, bool isOctet, bool isUnsigned, Emitter output)
{
    if ((isOctet && isToBigOctal(val)) || (isHex && isToBigHex(val)) || (isToBigDec(val)))
    {
        output->emit_invalid(data);
        return;
    } 
    else if (val[0] == '-')
    {
        if (isUnsigned) output->emit_invalid(data);
        else emitLiteral<T>(data, PA2Decode_ll(val, isHex, isOctet), false, output);
    }
    else emitLiteral<T, unsigned long long int>(data, PA2Decode_ull(val, isHex, isOctet), isUnsigned, output);
}
template <typename Emitter>
void emitDoubleLiteral(const std::string& data, Emitter output)
{
    double aux = PA2Decode_double(data);
    output->emit_literal(data, EFundamentalType::FT_DOUBLE, &aux, sizeof(aux));
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

    auto dots = std::count(prefix.begin(), prefix.end(), '.');

    if (dots == 0)
    {
        if (suffix.empty())
            emitLiteral<int>(data, value, isHex, isOctet, isUnsigned, output);
        else if (matches(longSuffix, suffix))
            emitLiteral<long int>(data, value, isHex, isOctet, isUnsigned, output);
        else if (matches(longLongSuffix, suffix))
            emitLiteral<long long int>(data, value, isHex, isOctet, isUnsigned, output);
        else if (matches(unsignedSuffix, suffix))
            emitLiteral<unsigned int>(data, value, isHex, isOctet, true, output);
        else if (matches(unsignedLongSuffix, suffix))
            emitLiteral<unsigned long int>(data, value, isHex, isOctet, true, output);
        else if (matches(unsignedLongLongSuffix, suffix))
            emitLiteral<unsigned long long int>(data, value, isHex, isOctet, true, output);
        else if (matches(eNotation, suffix))
            emitDoubleLiteral(data, output);
        else if (matches(userDefinedSuffix, suffix))
            output->emit_user_defined_literal_integer(data, suffix, (isHex ? "0x" : (isOctet ? "0" : "")) + prefix);
        else output->emit_invalid(data);
    }
    else if ((dots < 2) && (suffix.empty() || matches(eNotation, suffix)))
        emitDoubleLiteral(data, output);
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

std::string convertEscapeSeq(std::string arg)
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
        return splitWideChar(toWideChar(aux));
    }
    else if ((arg.size() < 17) && matches(+chset(L"0-7"), arg.substr(1)))
    {
        uint32_t aux = 0;
        for (auto c : arg.substr(1)) aux = (aux << 3) + int(c - '0');
        return splitWideChar(toWideChar(aux));
    }
    return "";
}

void PostTokenAnalyser::emit_character_literal(const string& data)
{
    auto index = data.find('\'');
    auto prefix = data.substr(0, index);
    auto str = convertEscapeSeq(data.substr(index + 1, data.size() - index - 2));
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

void PostTokenAnalyser::emit_user_defined_character_literal(const string& data)
{
}

void PostTokenAnalyser::emit_string_literal(const string& data)
{
}

void PostTokenAnalyser::emit_user_defined_string_literal(const string& data)
{
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

