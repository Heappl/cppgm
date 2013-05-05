#ifndef STATE_MACHINE_HPP
#define STATE_MACHINE_HPP

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

#endif //STATE_MACHINE_HPP

