#include "StateMachine.hpp"
#include <cassert>
#include <deque>

namespace
{
typedef std::pair<unsigned, unsigned> CharRange;
typedef std::set<CharRange> CharRanges;
typedef std::pair<CharRanges, State*> Transition;
typedef std::vector<Transition> Transitions;
typedef std::vector<State*> AutomataEndings;
typedef std::pair<State*, AutomataEndings> AutomataUnderConstruction;
typedef std::vector<AutomataUnderConstruction> AutomatasUnderConstruction;

} //namespace

struct State
{
    Transitions transitions;
};

namespace
{


AutomataUnderConstruction createAutomata(const Rule& rule);
AutomataUnderConstruction createOrAutomata(const Rule& rule)
{
    assert(rule.discriminator == Rule::Type::DOrSeq);
    AutomatasUnderConstruction automatas;
    for (auto subrule : rule.subrules)
    {
        automatas.push_back(createAutomata(subrule));
    }
    State* start = new State();
    AutomataEndings endings;
    for (auto automata : automatas)
    {
        endings.insert(endings.end(), automata.second.begin(), automata.second.end());
        start->transitions.push_back(Transition({}, automata.first));
    }
    return AutomataUnderConstruction(start, endings);
}
AutomataUnderConstruction createRepeatAutomata(const Rule& rule)
{
    assert(rule.discriminator == Rule::Type::DRepeat);
    assert(rule.subrules.size() == 1);
    AutomataUnderConstruction subautomata = createAutomata(rule.subrules.front());
    for (auto ending : subautomata.second)
    {
        Transition endingTransition = ending->transitions.front();
        endingTransition.second = subautomata.first;
        ending->transitions.push_back(endingTransition);
    }
    auto startState = new State();
    startState->transitions.push_back(Transition({}, nullptr));
    startState->transitions.push_back(Transition({}, subautomata.first));
    subautomata.second.push_back(startState);
    return AutomataUnderConstruction(startState, subautomata.second);
}
AutomataUnderConstruction createSeqAutomata(const Rule& rule)
{
    assert(rule.discriminator == Rule::Type::DSeq);
    AutomatasUnderConstruction automatas;
    for (auto subrule : rule.subrules)
    {
        automatas.push_back(createAutomata(subrule));
    }
    AutomataUnderConstruction prev = automatas.front();
    for (auto automataIt = automatas.begin() + 1; automataIt != automatas.end(); ++automataIt)
    {
        for (auto ending : prev.second)
            ending->transitions.front().second = automataIt->first;
        prev = *automataIt;
    }
    return AutomataUnderConstruction(automatas.front().first, prev.second);
}
AutomataUnderConstruction createChsetAutomata(const Rule& rule)
{
    assert(rule.discriminator == Rule::Type::DChset);
    assert(rule.subrules.empty());
    State* start = new State();
    start->transitions.push_back({rule.chset.ranges, nullptr});
    return AutomataUnderConstruction(start, {start});
}
AutomataUnderConstruction createAutomata(const Rule& rule)
{
    switch (rule.discriminator)
    {
        case Rule::Type::DChset:
            return createChsetAutomata(rule);
        case Rule::Type::DSeq:
            return createSeqAutomata(rule);
        case Rule::Type::DRepeat:
            return createRepeatAutomata(rule);
        case Rule::Type::DOrSeq:
            return createOrAutomata(rule);
        default:
            return AutomataUnderConstruction(nullptr, {});
    }
}

} //namespace

namespace
{
typedef std::set<State*> StateSet;

bool matches(const CharRanges& ranges, wchar_t c)
{
    unsigned temp = c;
    for (auto range : ranges)
    {
        if ((temp >= range.first) && (temp <= range.second))
            return true;
    }
    return false;
}

bool processEpsilonTransitions(StateSet& states)
{
    StateSet nextStates;
    bool matched = false;
    std::deque<State*> stateQueue(states.begin(), states.end());
    while (not stateQueue.empty())
    {
        auto curr = stateQueue.front();
        stateQueue.pop_front();
        for (auto transition : curr->transitions)
        {
            if (transition.first.empty())
            {
                if (transition.second == nullptr) matched = true;
                else if (nextStates.count(transition.second) == 0)
                    stateQueue.push_back(transition.second);
            }
        }
        nextStates.insert(curr);
    }
    states = nextStates;
    return matched;
}

bool process(StateSet& states, wchar_t c)
{
    StateSet nextStates;
    bool matched = false;
    for (auto state : states)
    {
        for (auto transition : state->transitions)
        {
            if (matches(transition.first, c))
            {
                if (transition.second == nullptr) matched = true;
                else nextStates.insert(transition.second);
            }
        }
    }
    states = nextStates;
    matched |= processEpsilonTransitions(states);
    return matched;
}

} //namespace

StateMachine::StateMachine()
{
}

StateMachine::StateMachine(const Rule& rule)
{
    states.insert(createAutomata(rule).first);
    processEpsilonTransitions(states);
}

bool StateMachine::process(wchar_t c)
{
    return ::process(states, c);
}

bool StateMachine::foreverUnmatched()
{
    return states.empty();
}

