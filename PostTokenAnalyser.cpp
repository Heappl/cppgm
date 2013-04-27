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

template <typename T, typename Emitter>
void emitLiteral(const std::string& data, const string& val, bool isHex, bool isOctet, Emitter output)
{
    auto aux = T(PA2Decode_long_long(val, isHex, isOctet));
    std::cerr << isHex << " " << data << ": " << std::hex << aux << std::dec << std::endl;
    output->emit_literal(data, FundamentalTypeOf<T>(), &aux, sizeof(aux));
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
            emitLiteral<int>(data, prefix, isHex, isOctet, output);
        else if (matches(longSuffix, suffix))
            emitLiteral<long int>(data, prefix, isHex, isOctet, output);
        else if (matches(longLongSuffix, suffix))
            emitLiteral<long long int>(data, prefix, isHex, isOctet, output);
        else if (matches(unsignedSuffix, suffix))
            emitLiteral<unsigned int>(data, prefix, isHex, isOctet, output);
        else if (matches(unsignedLongSuffix, suffix))
            emitLiteral<unsigned long int>(data, prefix, isHex, isOctet, output);
        else if (matches(unsignedLongLongSuffix, suffix))
            emitLiteral<unsigned long long int>(data, prefix, isHex, isOctet, output);
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

void PostTokenAnalyser::emit_character_literal(const string& data)
{
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

