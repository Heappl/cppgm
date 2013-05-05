#ifndef REGEX_RULE_HPP
#define REGEX_RULE_HPP

#include <vector>
#include <utility>
#include <set>
#include <string>
#include <stdexcept>
#include <limits>

#define MAX_ANYCHAR (0xFFFFFFFF)
#define MIN_ANYCHAR (0)

struct Rule;
typedef std::vector<Rule> Rules;
typedef std::pair<uint64_t, uint64_t> CharRange;
typedef std::set<CharRange> CharRanges;

struct RuleException : std::runtime_error
{
    RuleException(std::string what) : std::runtime_error(what) {}
};

struct Chset
{
    CharRanges ranges;

    Chset(std::wstring chset = L"");
    Chset(std::vector<std::pair<int, int>> intRanges);
    Chset operator~() const;
    Chset operator-(const Chset& other) const;
};

Chset operator|(const Chset& left, const Chset& right);
Rule chset(std::wstring chset);
Rule chset(wchar_t c);

struct Rule
{
    enum class Type
    {
        DRepeat,
        DOrSeq,
        DSeq,
        DChset,
        DEmpty
    };
    Type discriminator;
    Rules subrules;
    Chset chset;

    Rule(Type d);
    Rule(Chset chset);
    Rule(Type d, Rules subrules);
    Rule(const std::wstring& text);
    Rule(const wchar_t* text);

    Rule operator>>(const Rule& other) const;
    Rule operator+() const;
    Rule operator*() const;
    Rule operator|(const Rule& other) const;
    Rule operator~() const;
    Rule operator-(const Rule& other) const;
    Rule operator!() const;
};

Rule operator>>(const wchar_t* left, const Rule& right);
Rule operator|(const wchar_t* left, const Rule& right);
Rule chset(wchar_t c);
Rule chset(uint64_t);
Rule chset(std::vector<uint64_t>);
Rule chset(std::wstring chset);
extern Rule anychar;
Rule chseq(std::wstring seq);
Rule strset(std::vector<std::wstring> strs);
Rule chsetFromRanges(std::vector<std::pair<int, int>> ranges);

#endif //REGEX_RULE_HPP

