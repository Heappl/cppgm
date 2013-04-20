#ifndef LEXER_HPP
#define LEXER_HPP

#include "StateMachine.hpp"
#include "RegexRule.hpp"

struct InvalidChar : std::runtime_error
{
    std::wstring text;
    InvalidChar(std::wstring text) : std::runtime_error("invalid character"), text(text) {}
};
template <typename TokenType>
struct IncompleteToken : std::runtime_error
{
    std::wstring tokenText;
    TokenType token;
    IncompleteToken(std::wstring tokenText, TokenType token)
        : std::runtime_error("incomplete token"), tokenText(tokenText), token(token)
    {}
};


template <typename TokenType>
struct TokenizerMachine
{
    typedef std::pair<unsigned, TokenType> TokenCandidate;
    typedef std::vector<StateMachine> StateMachines;
    typedef std::pair<StateMachines, TokenCandidate> OngoingToken;
    typedef std::map<unsigned, OngoingToken> OngoingTokens;
    typedef std::function<void(TokenType, std::wstring)> TokenHandler;
    typedef std::pair<StateMachine, TokenType> TokenMatcher;
    typedef std::vector<TokenMatcher> TokenMatchers;

    OngoingTokens ongoing;
    StateMachines startingMachines;
    std::vector<TokenType> tokens;
    TokenHandler handler;
    std::wstring buffer;
    unsigned lastEmittedEnd;
    unsigned processed;

    TokenizerMachine(TokenMatchers matchers, TokenHandler handler)
        : startingMachines(matchers.size()),
          tokens(matchers.size()),
          handler(handler),
          lastEmittedEnd(0),
          processed(0)
    {
        for (unsigned i = 0; i < matchers.size(); ++i)
        {
            startingMachines[i] = matchers[i].first;
            tokens[i] = matchers[i].second;
        }
    }

    void process(wchar_t c, std::wstring actualText)
    {
        buffer += actualText;
        processed += actualText.size();
        ongoing[processed - 1] = {startingMachines, {0, TokenType()}};
        
        auto it = ongoing.begin();
        while (it != ongoing.end())
        {
            bool neverWillMatchAgain = true;
            StateMachines& curr = it->second.first;
            TokenType tokenSoFar = TokenType();
            for (unsigned j = 0; j < curr.size(); ++j)
            {
                if (curr[j].foreverUnmatched()) continue;
                if (curr[j].process(c))
                {
                    it->second.second = TokenCandidate(processed, tokens[j]);
                    ongoing.erase(ongoing.lower_bound(it->first + 1), ongoing.upper_bound(processed));
                }
                if (not curr[j].foreverUnmatched())
                {
                    neverWillMatchAgain = false;
                    tokenSoFar = tokens[j];
                }
            }
            if (neverWillMatchAgain && (it == ongoing.begin()))
            {
                auto tokenCand = it->second.second;
                if (tokenCand.first == 0)
                {
                    if (processed - lastEmittedEnd <= 1) throw InvalidChar(buffer);
                    else throw IncompleteToken<TokenType>(buffer, tokenSoFar);
                }
                handler(tokenCand.second, buffer.substr(0, tokenCand.first - lastEmittedEnd));
                buffer = buffer.substr(tokenCand.first - lastEmittedEnd);
                lastEmittedEnd = tokenCand.first;
                auto toErase = it;
                ++it;
                ongoing.erase(toErase);
            }
            else ++it;
        }
    }
};

template <typename InType, typename OutType>
struct TokenizerChainLink
{
    typedef InType FirstType;
    typedef OutType SecondType;

    typedef std::function<void(OutType, std::wstring)> NextHandler;
    typedef std::pair<OutType, Rule> TokenDefinition;
    typedef std::vector<TokenDefinition> TokenDefinitions;
    typedef std::pair<StateMachine, OutType> TokenMatcher;
    typedef std::vector<TokenMatcher> TokenMatchers;

    TokenizerMachine<OutType> machine;

    TokenizerChainLink(NextHandler handler, TokenDefinitions defs)
        : machine(createMatchers(defs), handler)
    {}

    void process(const InType& token, std::wstring actualText)
    {
        machine.process(wchar_t(token), actualText);
    }

    TokenMatcher createMatcher(const TokenDefinition& definition)
    {
        return TokenMatcher(StateMachine(definition.second), definition.first);
    }
    TokenMatchers createMatchers(const TokenDefinitions& definitions)
    {
        TokenMatchers out;
        for (auto definition : definitions)
            out.push_back(createMatcher(definition));
        return out;
    }
};

template <typename PrevOut, typename In, typename Out>
struct TokenizerChainLinkType
{
    typedef TokenizerChainLink<In, Out> type;
    typedef PrevOut PrevOutType;
};

template <typename TokenizerChainLink, typename... OtherLinks>
struct TokenizerChain : TokenizerChain<OtherLinks...>
{
    typedef typename TokenizerChainLink::type::FirstType InType;
    typedef typename TokenizerChainLink::type::SecondType OutType;
    typedef typename TokenizerChainLink::PrevOutType PrevOutType;

    typedef std::pair<OutType, Rule> Definition;
    typedef std::vector<Definition> Definitions;
    typedef std::function<void(PrevOutType, std::wstring)> LinkHandler;
    typedef std::function<void(InType, std::wstring)> NextLinkHandler;
    typedef std::function<void(PrevOutType, std::wstring, NextLinkHandler)> PhaseLinker;

    typename TokenizerChainLink::type link;
    LinkHandler handler;

    template <typename FinalHandler, typename... TailDefinitions>
    TokenizerChain(
        FinalHandler finalLinkHandler,
        const std::pair<PhaseLinker, Definitions>& thisLinkData,
        const TailDefinitions& ...tailLinksData)
        : TokenizerChain<OtherLinks...>(finalLinkHandler, tailLinksData...),
          link(TokenizerChain<OtherLinks...>::handler, thisLinkData.second),
          handler([&](PrevOutType token, std::wstring text) {
             thisLinkData.first(token, text, [&](InType token, std::wstring text){ link.process(token, text); }); })
    {}
};
template <typename TokenizerChainLink>
struct TokenizerChain<TokenizerChainLink>
{
    typedef typename TokenizerChainLink::type::FirstType InType;
    typedef typename TokenizerChainLink::type::SecondType OutType;
    typedef typename TokenizerChainLink::PrevOutType PrevOutType;

    typedef std::pair<OutType, Rule> Definition;
    typedef std::vector<Definition> Definitions;
    typedef std::function<void(PrevOutType, std::wstring)> LinkHandler;
    typedef std::function<void(InType, std::wstring)> NextLinkHandler;
    typedef std::function<void(PrevOutType, std::wstring, NextLinkHandler)> PhaseLinker;

    typename TokenizerChainLink::type link;
    LinkHandler handler;

    template <typename PhaseLinker, typename FinalLinkHander>
    TokenizerChain(
        FinalLinkHander finalLinkHandler,
        const std::pair<PhaseLinker, Definitions>& linkData)
        : link(finalLinkHandler, linkData.second),
          handler([&](PrevOutType token, std::wstring text) {
             linkData.first(token, text, [&](InType token, std::wstring text){ link.process(token, text); }); })
    {}
};

#endif //LEXER_HPP

