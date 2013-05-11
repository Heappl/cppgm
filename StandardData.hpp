#pragma once

#include <vector>
#include <unordered_set>
#include <utility>
#include <string>
#include <map>
#include <unordered_map>
#include "ETokenType.h"
#include "EFundamentalType.h"

// EndOfFile: synthetic "character" to represent the end of source file
constexpr uint64_t EndOfFile = -1;

// See C++ standard 2.11 Identifiers and Appendix/Annex E.1
extern const std::vector<std::pair<int, int>> AnnexE1_Allowed_RangesSorted;

// See C++ standard 2.11 Identifiers and Appendix/Annex E.2
extern const std::vector<std::pair<int, int>> AnnexE2_DisallowedInitially_RangesSorted;

// See C++ standard 2.13 Operators and punctuators
extern const std::unordered_set<std::string> Digraph_IdentifierLike_Operators;

// See `simple-escape-sequence` grammar
extern const std::unordered_set<int> SimpleEscapeSequence_CodePoints;


// FundamentalTypeOf: convert fundamental type T to EFundamentalType
// for example: `FundamentalTypeOf<long int>()` will return `FT_LONG_INT`
template<typename T> constexpr EFundamentalType FundamentalTypeOf();
template<> constexpr EFundamentalType FundamentalTypeOf<signed char>() { return FT_SIGNED_CHAR; }
template<> constexpr EFundamentalType FundamentalTypeOf<short int>() { return FT_SHORT_INT; }
template<> constexpr EFundamentalType FundamentalTypeOf<int>() { return FT_INT; }
template<> constexpr EFundamentalType FundamentalTypeOf<long int>() { return FT_LONG_INT; }
template<> constexpr EFundamentalType FundamentalTypeOf<long long int>() { return FT_LONG_LONG_INT; }
template<> constexpr EFundamentalType FundamentalTypeOf<unsigned char>() { return FT_UNSIGNED_CHAR; }
template<> constexpr EFundamentalType FundamentalTypeOf<unsigned short int>() { return FT_UNSIGNED_SHORT_INT; }
template<> constexpr EFundamentalType FundamentalTypeOf<unsigned int>() { return FT_UNSIGNED_INT; }
template<> constexpr EFundamentalType FundamentalTypeOf<unsigned long int>() { return FT_UNSIGNED_LONG_INT; }
template<> constexpr EFundamentalType FundamentalTypeOf<unsigned long long int>() { return FT_UNSIGNED_LONG_LONG_INT; }
template<> constexpr EFundamentalType FundamentalTypeOf<wchar_t>() { return FT_WCHAR_T; }
template<> constexpr EFundamentalType FundamentalTypeOf<char>() { return FT_CHAR; }
template<> constexpr EFundamentalType FundamentalTypeOf<char16_t>() { return FT_CHAR16_T; }
template<> constexpr EFundamentalType FundamentalTypeOf<char32_t>() { return FT_CHAR32_T; }
template<> constexpr EFundamentalType FundamentalTypeOf<bool>() { return FT_BOOL; }
template<> constexpr EFundamentalType FundamentalTypeOf<float>() { return FT_FLOAT; }
template<> constexpr EFundamentalType FundamentalTypeOf<double>() { return FT_DOUBLE; }
template<> constexpr EFundamentalType FundamentalTypeOf<long double>() { return FT_LONG_DOUBLE; }
template<> constexpr EFundamentalType FundamentalTypeOf<void>() { return FT_VOID; }
template<> constexpr EFundamentalType FundamentalTypeOf<std::nullptr_t>() { return FT_NULLPTR_T; }

// convert EFundamentalType to a source code
extern const std::map<EFundamentalType, std::string> FundamentalTypeToStringMap;

// StringToETokenTypeMap map of `simple` `preprocessing-tokens` to ETokenType
extern const std::unordered_map<std::string, ETokenType> StringToTokenTypeMap;

// map of enum to string
extern const std::map<ETokenType, std::string> TokenTypeToStringMap;

template <typename T>
struct NumericProgression
{
    typedef T top_type;
    typedef T top_dec_type;
    typedef T top_unsigned_type;
    typedef T bottom_type;
};

template <>
struct NumericProgression<int>
{
    typedef unsigned int top_type;
    typedef long int top_dec_type;
    typedef unsigned int top_unsigned_type;
    typedef long int bottom_type;
};

template <>
struct NumericProgression<unsigned int>
{
    typedef long int top_type;
    typedef long int top_dec_type;
    typedef unsigned long int top_unsigned_type;
    typedef int bottom_type;
};

template <>
struct NumericProgression<long int>
{
    typedef unsigned long int top_type;
    typedef long long int top_dec_type;
    typedef unsigned long int top_unsigned_type;
    typedef long long int bottom_type;
};

template <>
struct NumericProgression<unsigned long int>
{
    typedef long long int top_type;
    typedef long long int top_dec_type;
    typedef unsigned long long int top_unsigned_type;
    typedef int bottom_type;
};

template <>
struct NumericProgression<long long int>
{
    typedef unsigned long long int top_type;
    typedef long long int top_dec_type;
    typedef unsigned long long int top_unsigned_type;
    typedef long long int bottom_type;
};

template <>
struct NumericProgression<unsigned long long int>
{
    typedef unsigned long long int top_dec_type;
    typedef unsigned long long int top_type;
    typedef unsigned long long int top_unsigned_type;
    typedef int bottom_type;
};

