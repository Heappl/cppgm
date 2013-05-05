#include "PPTokenizer.hpp"
#include "StandardData.hpp"
#include "Helpers.hpp"
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <utility>
#include <set>
#include <cassert>
#include <deque>
#include <map>
#include <functional>
#include <functional>
#include "RegexRule.hpp"
#include "StateMachine.hpp"
#include "Lexer.hpp"

namespace {

constexpr wchar_t CommentBegin = wchar_t(-2);
constexpr wchar_t StartOfFile = wchar_t(-3);

enum class PrephaseToken
{
    Trigraph,
    RawString,
    PlainCharacter
};

enum class Token
{
    PPNumber,
    CharacterLiteral,
    Identifier,
    WhiteSpace,
    Newline,
    PreprocessingOpOrPunc,
    NonWhitespaceCharacter,
    SpecialPreprocessingOpOrPuncSeq,
    StringLiteral,
    Ignore,
    HeaderInclude
};
enum class FirstPhaseToken
{
    PlainCharacter,
    MultiLineCommentStart,
    UniversalCharacterTuple,
    UniversalCharacter,
    EscapedLineBreak
};
enum class CommentPhaseToken
{
    PlainCharacter,
    SingleLineComment,
    MultiLineComment,
    StringLiteral,
    CharacterLiteral
};

typedef TokenizerChain<
    TokenizerChainLinkType<wchar_t, wchar_t, PrephaseToken>,
    TokenizerChainLinkType<PrephaseToken, wchar_t, FirstPhaseToken>,
    TokenizerChainLinkType<FirstPhaseToken, wchar_t, CommentPhaseToken>,
    TokenizerChainLinkType<CommentPhaseToken, wchar_t, Token>> ChainedTokenizer;

typedef std::function<void (wchar_t, std::wstring)> Handler;
typedef std::function<void(wchar_t, std::wstring, Handler)> InitialLinker;
typedef std::function<void(PrephaseToken, std::wstring, Handler)> PreToFirstPhaseLinker;
typedef std::function<void(FirstPhaseToken, std::wstring, Handler)> FirstToCommentPhaseLinker;
typedef std::function<void(CommentPhaseToken, std::wstring, Handler)> CommentToSecondPhaseLinker;
typedef std::vector<std::pair<PrephaseToken, Rule>> PrephaseDefinitions;
typedef std::vector<std::pair<FirstPhaseToken, Rule>> FirstPhaseDefinitions;
typedef std::vector<std::pair<CommentPhaseToken, Rule>> CommentPhaseDefinitions;
typedef std::vector<std::pair<Token, Rule>> SecondPhaseDefinitions;

void tokenHandler(Token token, std::wstring val, std::shared_ptr<IPPTokenStream>& output)
{
    if (token == Token::CharacterLiteral)
    {
        if (val.back() != '\'')
            output->emit_user_defined_character_literal(toStr(val));
        else output->emit_character_literal(toStr(val));
    }
    else if (token == Token::PPNumber) output->emit_pp_number(toStr(val));
    else if (token == Token::Identifier) {
        output->emit_identifier(toStr(val));
    }
    else if (token == Token::WhiteSpace) output->emit_whitespace_sequence();
    else if (token == Token::PreprocessingOpOrPunc) output->emit_preprocessing_op_or_punc(toStr(val));
    else if (token == Token::NonWhitespaceCharacter) output->emit_non_whitespace_char(toStr(val));
    else if (token == Token::StringLiteral) {
        if (val.back() != '"')
            output->emit_user_defined_string_literal(toStr(val));
        else output->emit_string_literal(toStr(val));
    }
    else if (token == Token::Newline)
    {
        output->emit_new_line();
    }
    else if (token == Token::SpecialPreprocessingOpOrPuncSeq)
    {
        if (val == L"<::")
        {
            output->emit_preprocessing_op_or_punc("<");
            output->emit_preprocessing_op_or_punc("::");
        } else if (val == L"<::>") {
            output->emit_preprocessing_op_or_punc("<:");
            output->emit_preprocessing_op_or_punc(":>");
        } else if (val == L"<:::") {
            output->emit_preprocessing_op_or_punc("<:");
            output->emit_preprocessing_op_or_punc("::");
        }
    }
    else if (token == Token::HeaderInclude)
    {
        if (val[0] == L'\n') output->emit_new_line();
        output->emit_preprocessing_op_or_punc("#");
        output->emit_identifier("include");
        output->emit_whitespace_sequence();
        output->emit_header_name(toStr(val.substr(val.find_first_of(L"\"<"))));
    }
}

void commentAndSecondPhaseLinker(CommentPhaseToken t, std::wstring text, Handler h)
{
    if (t == CommentPhaseToken::PlainCharacter) h(text[0], text);
    if (t == CommentPhaseToken::MultiLineComment) h(' ', text);
    if (t == CommentPhaseToken::SingleLineComment) h(' ', text);
    if ((t == CommentPhaseToken::StringLiteral) || (t == CommentPhaseToken::CharacterLiteral))
        for (auto c : text) h(c, {c});
}
void firstAndCommentPhaseLinker(FirstPhaseToken t, std::wstring text, Handler h)
{
    if (t == FirstPhaseToken::PlainCharacter) h(text[0], text);
    if (t == FirstPhaseToken::MultiLineCommentStart) h(CommentBegin, text);
    if (t == FirstPhaseToken::UniversalCharacterTuple)
    {
        int aux = 0;
        for (unsigned i = 2; i < text.size(); ++i)
            aux = (aux << 4) + HexCharToValue(text[i]);
        wchar_t c = toWideCharInUtf8(aux);
        h(c, {c});
    }
    if (t == FirstPhaseToken::UniversalCharacter)
    {
        int aux = 0;
        for (unsigned i = 0; i < text.size(); ++i)
            aux = (aux << 8) + int(text[i]);
        wchar_t c = wchar_t(aux);
        h(c, {c});
    }
}

void prephaseLinker(PrephaseToken t, std::wstring text, Handler h)
{
    if (t == PrephaseToken::PlainCharacter) h(text[0], text);
    if (t == PrephaseToken::Trigraph)
    {
        switch(text[text.size() - 1])
        {
            case L'=': h(L'#', L"#"); break;
            case L'/': h(L'\\', L"\\"); break;
            case L'\'': h(L'^', L"^"); break;
            case L'(': h(L'[', L"["); break;
            case L')': h(L']', L"]"); break;
            case L'!': h(L'|', L"|"); break;
            case L'<': h(L'{', L"{"); break;
            case L'>': h(L'}', L"}"); break;
            case L'-': h(L'~', L"~"); break;
        }
    }
    if (t == PrephaseToken::RawString)
    {
        for (auto c : text) h(c, {c});
    }
}

std::vector<std::wstring> ops = {
    L"{", L"}", L"[", L"]", L"#", L"##", L"(", L")", L"<:", L":>", L"<%", L"%>", L"%:", L"%:%:",
    L";", L":", L"...", L"new", L"delete", L"?", L"::", L".", L".*", L"+", L"-", L"*", L"/", L"%",
    L"^", L"&", L"|", L"~", L"!", L"=", L"<", L">", L"+=", L"-=", L"*=", L"/=", L"%=", L"^=",
    L"&=", L"|=", L"<<", L">>", L">>=", L"<<=", L"==", L"!=", L"<=", L">=", L"&&", L"||", L"++",
    L"--", L",", L"->*", L"->", L"and", L"and_eq", L"bitand", L"bitor", L"compl", L"not",
    L"not_eq", L"or", L"or_eq", L"xor", L"xor_eq"
};
std::vector<std::wstring> stringLiteralsOpenQuotes = {
    L"\"", L"R\"", L"u8\"", L"u8R\"", L"u\"", L"uR\"", L"U\"", L"UR\"", L"L\"", L"LR\""
};
std::wstring stringLiteralPostfixChars = L" ()\\" + std::wstring({wchar_t(0), L'-', wchar_t(37)});

} //namespace

struct PPTokenizer::PPTokenizerImpl
{
    std::shared_ptr<IPPTokenStream> output;
    bool started;
    
    Rule identifierNondigitChar;
    Rule utf8TrailingSymbol;
    Rule asciiCompatibleChar;
    Rule firstIdentifierChar;
    Rule identifier;
    Rule hex;
    Rule hexquad;
    Rule usedChar;
    Rule startOfFile;
    Rule stringLiteralContent;
    Rule charLiteralContent;

    ChainedTokenizer tokenizerChain;

    PPTokenizerImpl(std::shared_ptr<IPPTokenStream> output)
        : output(output),
          started(false),
          identifierNondigitChar(chsetFromRanges(convertToWide(AnnexE1_Allowed_RangesSorted)) | chset(L"a-zA-Z_")),
          utf8TrailingSymbol(chsetFromRanges({{0x80, 0xbf}})),
          asciiCompatibleChar(chsetFromRanges({{0x00, 0x7f}})),
          firstIdentifierChar(identifierNondigitChar - chsetFromRanges(convertToWide(AnnexE2_DisallowedInitially_RangesSorted))),
          identifier(firstIdentifierChar >> *(identifierNondigitChar | chset(L"0-9"))),
          hex(chset(L"0-9a-fA-F")),
          hexquad(hex >> hex >> hex >> hex),
          usedChar(firstIdentifierChar | chset(L"-0-9~^{}[]#()<>:%;.?.+*/&|=!,\"' \n\t\v\r\b\f")),
          startOfFile({StartOfFile}),
          stringLiteralContent(*(~chset(L"\\\"") | (L"\\" >> anychar) | chset(CommentBegin))),
          charLiteralContent(*(~chset(L"\\'") | (L"\\" >> anychar) | chset(CommentBegin))),
          tokenizerChain(
              std::bind(tokenHandler, std::placeholders::_1, std::placeholders::_2, output),
              std::pair<InitialLinker, PrephaseDefinitions>(
                  [](wchar_t c, std::wstring text, Handler h) { h(c, text); },
                  {
                      {PrephaseToken::Trigraph, L"??" >> chset(L"=/'()!<>-")},
                      {PrephaseToken::RawString, L"R\"" >> stringLiteralContent >> L"\"" },
                      {PrephaseToken::PlainCharacter, (anychar | chset(wchar_t(EndOfFile)))}
                  }
              ),
              std::pair<PreToFirstPhaseLinker, FirstPhaseDefinitions>(
                  prephaseLinker,
                  {
                      {FirstPhaseToken::EscapedLineBreak, chseq(L"\\\n")},
                      {FirstPhaseToken::UniversalCharacterTuple, L"\\" >> ((L"u" >> hexquad) | (L"U" >> hexquad >> hexquad))},
                      {FirstPhaseToken::UniversalCharacter,
                          (chsetFromRanges({{0xf0, 0xf7}}) >> utf8TrailingSymbol >> utf8TrailingSymbol >> utf8TrailingSymbol)
                          | (chsetFromRanges({{0xe0, 0xef}}) >> utf8TrailingSymbol >> utf8TrailingSymbol)
                          | (chsetFromRanges({{0xc0, 0xdf}}) >> utf8TrailingSymbol)},
                      {FirstPhaseToken::MultiLineCommentStart, chseq(L"/*")},
                      {FirstPhaseToken::PlainCharacter, (anychar | chset(wchar_t(EndOfFile)))}
                  }
              ),
              std::pair<FirstToCommentPhaseLinker, CommentPhaseDefinitions>(
                  firstAndCommentPhaseLinker,
                  {
                      {CommentPhaseToken::SingleLineComment, L"//" >> *~chset(L"\n")},
                      {CommentPhaseToken::MultiLineComment, chset(CommentBegin) >> *(~chset(L"*") | (L"*" >> ~chset(L"/"))) >> L"*/"},
                      {CommentPhaseToken::StringLiteral, L"\"" >> stringLiteralContent >> L"\""},
                      {CommentPhaseToken::CharacterLiteral, L"'" >> charLiteralContent >> L"'"},
                      {CommentPhaseToken::PlainCharacter, (anychar | chset(wchar_t(EndOfFile)))}
                  }
              ),
              std::pair<CommentToSecondPhaseLinker, SecondPhaseDefinitions>(
                  commentAndSecondPhaseLinker,
                  {
                      {Token::PPNumber, (chset(L"0-9") | (L"." >> chset(L"0-9")))
                                        >> *(*chset(L"a-zA-Z0-9._") >>  *(chset(L"eE") >> chset(L"-+")))},
                      {Token::Identifier, identifier},
                      {Token::CharacterLiteral,
                          ((chset(L"uUL") >> L"'") | L"'") >> charLiteralContent >> (L"'" | (L"'" >> identifier))},
                      {Token::WhiteSpace, *chset(L" \t\v\r\b\f")},
                      {Token::Newline, L"\n" | Rule({EndOfFile}) | (L"\n" >> Rule({EndOfFile}))},
                      {Token::HeaderInclude,
                          (L"\n" | startOfFile) >> L"#include" >> *chset(L" \t\v\r\b\f") >>
                              ((L"\"" >> *~chset(L"\"\n") >> L"\"") | (L"<" >> *~chset(L">\n") >> L">"))},
                      {Token::PreprocessingOpOrPunc, strset(ops)},
                      {Token::SpecialPreprocessingOpOrPuncSeq, strset({L"<::>", L"<::", L"<:::"})},
                      {Token::NonWhitespaceCharacter, ~usedChar - chset(wchar_t(0xff))},
                      {Token::Ignore, Rule({StartOfFile})},
                      {Token::StringLiteral,
                          strset(stringLiteralsOpenQuotes) >> stringLiteralContent >> (L"\"" | (L"\"" >> identifier))},
                  }
              )
          )
    { }

    void handle(int c)
    {
        tokenizerChain.handler(wchar_t(c), {wchar_t(c)});
    }
    void process(int c)
    {
        if (not started)
        {
            handle(int(StartOfFile));
            started = true;
            if (c == EndOfFile)
            {
                output->emit_eof();
                return;
            }
        }
        handle(c);
        if (c == EndOfFile)
        {
            output->emit_eof();
            return;
        }
    }
};

PPTokenizer::PPTokenizer(std::shared_ptr<IPPTokenStream> output)
    : output(output), impl(new PPTokenizerImpl(output))
{
}

void PPTokenizer::process(int c)
{
    impl->process(c);
}

