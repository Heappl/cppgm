#include "PostTokenAnalyser.hpp"
#include "StandardData.hpp"
#include "Helpers.hpp"
#include "RegexRule.hpp"
#include "StateMachine.hpp"
#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>

namespace
{

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

} //namespace

PostTokenAnalyser::PostTokenAnalyser(std::shared_ptr<IPostTokenStream> output)
    : output(output), stringProcessor(new StringLiteralsPostTokenProcessor(output))
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
    //ignoring
}

void PostTokenAnalyser::emit_identifier(const string& data)
{
    stringProcessor->flush();
    auto it = StringToTokenTypeMap.find(data);
    if (it != StringToTokenTypeMap.end())
        output->emit_simple(data, it->second);
    else
        output->emit_identifier(data);
}

void PostTokenAnalyser::emit_pp_number(const string& data)
{
    stringProcessor->flush();
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
    auto userDefinedSuffix = L"_" >> chset(L"a-zA-Z") >> *chset(L"a-zA-Z0-9_");
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

void PostTokenAnalyser::emit_character_literal(const string& data)
{
    stringProcessor->flush();
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

void PostTokenAnalyser::emit_user_defined_character_literal(const string& data)
{
    stringProcessor->flush();
    auto index = data.find_last_of("'");
    auto value = data.substr(1, index - 1);
    auto suffix = data.substr(index + 1);
    if (suffix.empty() || suffix[0] != '_')
        output->emit_invalid(data);
    else
        output->emit_user_defined_literal_character(
            data, suffix, FundamentalTypeOf<char>(), value.c_str(), value.size());
}

void PostTokenAnalyser::emit_string_literal(const string& data)
{
    stringProcessor->addString(data);
}

void PostTokenAnalyser::emit_user_defined_string_literal(const string& data)
{
    stringProcessor->addUserDefinedString(data);
}

void PostTokenAnalyser::emit_preprocessing_op_or_punc(const string& data)
{
    stringProcessor->flush();
    auto it = StringToTokenTypeMap.find(data);
    if (it != StringToTokenTypeMap.end())
        output->emit_simple(data, it->second);
    else
        output->emit_invalid(data);
}

void PostTokenAnalyser::emit_non_whitespace_char(const string& data)
{
    stringProcessor->flush();
    output->emit_invalid(data);
}

void PostTokenAnalyser::emit_eof()
{
    stringProcessor->flush();
    output->emit_eof();
}

