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
    bool process(wchar_t c);
    bool foreverUnmatched();
};

#endif //STATE_MACHINE_HPP

