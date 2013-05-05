#include "RegexRule.hpp"
#include "Helpers.hpp"
#include <iostream>
#include <limits>

Chset::Chset(std::wstring chset)
{
    unsigned index = 0;
    while (index < chset.size())
    {
        CharRange range = {wcharToUint64(chset[index]),
                           wcharToUint64(chset[index])};
        index++;
        if ((index < chset.size() - 1) && (chset[index] == L'-'))
        {
            ++index;
            range.second = wcharToUint64(chset[index]);
            ++index;
        }
        ranges.insert(range);
    }
}

Chset::Chset(std::vector<std::pair<int, int>> intRanges)
{
    for (auto range : intRanges)
        ranges.insert({range.first & MAX_ANYCHAR, range.second & MAX_ANYCHAR});
}

Chset Chset::operator~() const
{
    Chset out;
    uint64_t prev = MIN_ANYCHAR;
    for (auto range : ranges)
    {
        if ((range.first >= MIN_ANYCHAR) && (range.first - 1 >= prev))
            out.ranges.insert({prev, range.first - 1});
        prev = std::max(range.second + 1, prev);
    }
    if (prev <= MAX_ANYCHAR)
        out.ranges.insert({prev, MAX_ANYCHAR});
    return out;
}

Chset Chset::operator-(const Chset& other) const
{
    Chset out;
    auto it = other.ranges.begin();
    for (auto range : ranges)
    {
        while ((it != other.ranges.end()) && (range.first > it->second))
            ++it;
        if (it == other.ranges.end()) out.ranges.insert(range);
        else if (range.second < it->first) out.ranges.insert(range);
        else if ((it->first <= range.first) && (it->second >= range.second)) continue;
        else if ((it->first > range.first) && (it->second < range.second))
        {
            if (it->first != std::numeric_limits<decltype(it->first)>::min())
                out.ranges.insert({range.first, it->first - 1});
            if (it->second != std::numeric_limits<decltype(it->first)>::max())
                out.ranges.insert({it->second + 1, range.second});
        }
        else if (it->first > range.first)
        {
            if (it->first != std::numeric_limits<decltype(it->first)>::min())
                out.ranges.insert({range.first, it->first - 1});
        }
        else
        {
            if (it->second != std::numeric_limits<decltype(it->first)>::max())
                out.ranges.insert({it->second + 1, range.second});
        }
    }
    return out;
}

Chset operator|(const Chset& left, const Chset& right)
{
    Chset out;
    out.ranges.insert(left.ranges.begin(), left.ranges.end());
    out.ranges.insert(right.ranges.begin(), right.ranges.end());
    return out;
}

Rule::Rule(Type d) : discriminator(d) {}

Rule::Rule(Chset chset) : discriminator(Type::DChset), chset(chset) {}

Rule::Rule(Type d, Rules subrules) : discriminator(d), subrules(subrules) {}

Rule::Rule(const std::wstring& text) : discriminator(Type::DSeq)
{
    for (auto c : text) subrules.push_back(::chset(c));
}
Rule::Rule(const wchar_t* text) : discriminator(Type::DSeq)
{
    auto c = text;
    while (*c != '\0') subrules.push_back(::chset(*c++));
}

Rule Rule::operator>>(const Rule& other) const
{
    Rule out(Type::DSeq);
    if (discriminator == Type::DSeq) out = *this;
    else out.subrules.push_back(*this);
    out.subrules.push_back(other);
    return out;
}

Rule Rule::operator+() const
{
    return Rule(Type::DOrSeq, {**this});
}

Rule Rule::operator*() const
{
    return Rule(Type::DRepeat, {*this});
}

Rule Rule::operator|(const Rule& other) const
{
    if ((discriminator == Type::DChset)
        && (other.discriminator == Type::DChset))
        return Rule(chset | other.chset);
    Rule out(Type::DOrSeq);
    if (discriminator == Type::DOrSeq) out = *this;
    else out.subrules.push_back(*this);
    out.subrules.push_back(other);
    return out;
}

Rule Rule::operator~() const
{
    if (discriminator != Type::DChset)
        throw RuleException("only character sets can be negated");
    return Rule(~chset);
}
Rule Rule::operator-(const Rule& other) const
{
    if ((discriminator != Type::DChset) || (other.discriminator != Type::DChset))
        throw RuleException("subtract can be perform only on character sets");
    return Rule(chset - other.chset);
}
Rule Rule::operator!() const
{
    return *this | Rule(Type::DEmpty);
}

Rule operator>>(const wchar_t* left, const Rule& right)
{
    return Rule(left) >> right;
}

Rule operator|(const wchar_t* left, const Rule& right)
{
    return Rule(left) | right;
}

Rule chset(wchar_t c)
{
    return chset(wcharToUint64(c));
}

Rule chset(uint64_t arg)
{
    return chset(std::vector<uint64_t>({arg}));
}
Rule chset(std::vector<uint64_t> arg)
{
    Chset chset;
    for (auto elem : arg)
        chset.ranges.insert({elem, elem});
    return Rule(chset);
}

Rule chset(std::wstring chset)
{
    return Rule(Chset(chset));
}

Rule anychar = Rule(~Chset(L""));

Rule chseq(std::wstring seq)
{
    return Rule(seq);
}

Rule strset(std::vector<std::wstring> strs)
{
    Rules rules;
    for (auto str : strs)
        rules.push_back(chseq(str));
    return Rule(Rule::Type::DOrSeq, rules);
}

Rule chsetFromRanges(std::vector<std::pair<int, int>> ranges)
{
    return Rule(Chset(ranges));

}

