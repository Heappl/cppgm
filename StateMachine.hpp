#pragma once

#include "RegexRule.hpp"

struct State;
class StateMachine
{
    std::set<State*> states;
public:
    StateMachine();
    StateMachine(const Rule& rule);
    bool process(uint64_t c);
    bool foreverUnmatched();
};

bool matches(const Rule& rule, std::wstring str);
bool matches(const Rule& rule, std::string str);

