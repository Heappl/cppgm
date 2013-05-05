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
#include <limits>
#include "RegexRule.hpp"
#include "StateMachine.hpp"
#include "Lexer.hpp"

namespace {

constexpr uint64_t CommentBegin = std::numeric_limits<uint64_t>::max() - 1;
constexpr uint64_t StartOfFile = std::numeric_limits<uint64_t>::max() - 2;
constexpr uint64_t String = std::numeric_limits<uint64_t>::max() - 3;

enum class PrephaseToken
{
    Trigraph,
    RawString,
    PlainCharacter,
    StartOfFile,
    EndOfFile
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
    String,
    EndOfFile,
    StartOfFile,
    EscapedLineBreak
};
enum class CommentPhaseToken
{
    PlainCharacter,
    SingleLineComment,
    MultiLineComment,
    StringLiteral,
    String,
    EndOfFile,
    StartOfFile,
    CommentBegin,
    CharacterLiteral
};

typedef TokenizerChain<
    TokenizerChainLinkType<uint64_t, uint64_t, PrephaseToken>,
    TokenizerChainLinkType<PrephaseToken, uint64_t, FirstPhaseToken>,
    TokenizerChainLinkType<FirstPhaseToken, uint64_t, CommentPhaseToken>,
    TokenizerChainLinkType<CommentPhaseToken, uint64_t, Token>> ChainedTokenizer;

typedef std::function<void (uint64_t, std::wstring)> Handler;
typedef std::function<void(uint64_t, std::wstring, Handler)> InitialLinker;
typedef std::function<void(PrephaseToken, std::wstring, Handler)> PreToFirstPhaseLinker;
typedef std::function<void(FirstPhaseToken, std::wstring, Handler)> FirstToCommentPhaseLinker;
typedef std::function<void(CommentPhaseToken, std::wstring, Handler)> CommentToSecondPhaseLinker;
typedef std::vector<std::pair<PrephaseToken, Rule>> PrephaseDefinitions;
typedef std::vector<std::pair<FirstPhaseToken, Rule>> FirstPhaseDefinitions;
typedef std::vector<std::pair<CommentPhaseToken, Rule>> CommentPhaseDefinitions;
typedef std::vector<std::pair<Token, Rule>> SecondPhaseDefinitions;

void tokenHandler(Token token, std::wstring val, std::shared_ptr<IPPTokenStream>& output)
{
    //std::wcerr << L"tokenHandler: " << std::hex << uint64_t(token) << L" " << val << std::endl;
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
    //std::wcerr << L"commentAndSecondPhaseLinker: " << std::hex << uint64_t(t) << L" " << uint64_t(text[0]) << std::endl;
    if (t == CommentPhaseToken::PlainCharacter) h(wcharToUint64(text[0]), text);
    if (t == CommentPhaseToken::MultiLineComment) h(' ', text);
    if (t == CommentPhaseToken::SingleLineComment) h(' ', text);
    if (t == CommentPhaseToken::CharacterLiteral)
        for (auto c : text) h(wcharToUint64(c), {c});
    if ((t == CommentPhaseToken::StringLiteral) || (t == CommentPhaseToken::String))
        h(String, text);
    if (t == CommentPhaseToken::StartOfFile) h(StartOfFile, text);
    if (t == CommentPhaseToken::EndOfFile) h(EndOfFile, text);
    if (t == CommentPhaseToken::CommentBegin) h(CommentBegin, text);
}
void firstAndCommentPhaseLinker(FirstPhaseToken t, std::wstring text, Handler h)
{
    //std::wcerr << L"firstAndCommentPhaseLinker: " << std::hex << uint64_t(t) << L" " << text << std::endl;
    if (t == FirstPhaseToken::PlainCharacter) h(wcharToUint64(text[0]), text);
    if (t == FirstPhaseToken::MultiLineCommentStart) h(CommentBegin, text);
    if (t == FirstPhaseToken::UniversalCharacterTuple)
    {
        uint64_t aux = 0;
        for (unsigned i = 2; i < text.size(); ++i)
            aux = (aux << 4) + HexCharToValue(text[i]);
        wchar_t c = toWideCharInUtf8(aux);
        //std::wcerr << text << L": " << std::hex << wcharToUint64(c) << L" " << std::wstring({c}) << std::endl;
        h(wcharToUint64(c), {c});
    }
    if (t == FirstPhaseToken::UniversalCharacter)
    {
        uint64_t aux = 0;
        for (unsigned i = 0; i < text.size(); ++i)
            aux = (aux << 8) + wcharToUint64(text[i]);
        h(aux, {wchar_t(aux)});
    }
    if (t == FirstPhaseToken::String) h(String, text);
    if (t == FirstPhaseToken::StartOfFile) h(StartOfFile, text);
    if (t == FirstPhaseToken::EndOfFile) h(EndOfFile, text);
}

struct RawStringProcessor
{
    const std::wstring invalid;

    bool gettingOpening;
    bool gettingClosing;
    std::wstring rawStringId;
    std::wstring rawString;
    unsigned lastMatchedRawStringIdIndex;

    RawStringProcessor()
        : invalid(L"() \\\n\t\v\r\b\f"),
          gettingOpening(true),
          gettingClosing(false),
          rawString(L"R\""),
          lastMatchedRawStringIdIndex(0)
    {}
    
    void reset()
    {
        gettingClosing = false;
        gettingOpening = true;
        lastMatchedRawStringIdIndex = 0;
        rawString = L"R\"";
        rawStringId = L"";
    }

    bool process(uint64_t c)
    {
        rawString += {wchar_t(c)};
        //std::wcerr << L"rawString: " << rawString << L" " << rawStringId
                    //<< L" " << gettingOpening << L" " << gettingClosing
                    //<< L" " << std::hex << c << std::endl;
        if (c == EndOfFile) throw IncompleteToken<PrephaseToken>(rawString, PrephaseToken::RawString);
        else if (gettingOpening)
        {
            if (c == '(')
            {
                gettingOpening = false;
                rawStringId += L"\"";
            }
            else if (invalid.find(c) != std::string::npos) throw InvalidChar({wchar_t(c)});
            else rawStringId += {wchar_t(c)};
        }
        else if (gettingClosing)
        {
            if ((lastMatchedRawStringIdIndex >= rawStringId.size())
                || (rawStringId[lastMatchedRawStringIdIndex] != c))
            {
                lastMatchedRawStringIdIndex = 0;
                gettingClosing = false;
            }
            else ++lastMatchedRawStringIdIndex;
            if (lastMatchedRawStringIdIndex == rawStringId.size())
                return true;
        }
        else if (c == ')') gettingClosing = true;
        return false;
    }
};

void prephaseLinker(PrephaseToken t, std::wstring text, Handler h)
{
    static RawStringProcessor rawStringProcessor;
    static bool isRawString = false;
    //std::wcerr << L"prephaseLinker: " << std::hex << uint64_t(t) << L" " << text << L" " << isRawString << std::endl;

    if (isRawString)
    {
        bool finished = false;
        for (auto c : text) finished = rawStringProcessor.process(c);
        if (finished)
        {
            h(String, rawStringProcessor.rawString);
            isRawString = false;
        }
    }
    else if (t == PrephaseToken::PlainCharacter)
    {
        h(wcharToUint64(text[0]), text);
    }
    else if (t == PrephaseToken::Trigraph)
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
    else if (t == PrephaseToken::RawString)
    {
        isRawString = true;
        rawStringProcessor.reset();
    }
    else if (t == PrephaseToken::StartOfFile) h(StartOfFile, text);
    else if (t == PrephaseToken::EndOfFile) h(EndOfFile, text);
}

std::vector<std::wstring> ops = {
    L"{", L"}", L"[", L"]", L"#", L"##", L"(", L")", L"<:", L":>", L"<%", L"%>", L"%:", L"%:%:",
    L";", L":", L"...", L"new", L"delete", L"?", L"::", L".", L".*", L"+", L"-", L"*", L"/", L"%",
    L"^", L"&", L"|", L"~", L"!", L"=", L"<", L">", L"+=", L"-=", L"*=", L"/=", L"%=", L"^=",
    L"&=", L"|=", L"<<", L">>", L">>=", L"<<=", L"==", L"!=", L"<=", L">=", L"&&", L"||", L"++",
    L"--", L",", L"->*", L"->", L"and", L"and_eq", L"bitand", L"bitor", L"compl", L"not",
    L"not_eq", L"or", L"or_eq", L"xor", L"xor_eq"
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
          startOfFile(chset(StartOfFile)),
          stringLiteralContent(*(~chset(L"\\\"") | (L"\\" >> anychar) | chset(CommentBegin))),
          charLiteralContent(*(~chset(L"\\'") | (L"\\" >> anychar) | chset(CommentBegin))),
          tokenizerChain(
              std::bind(tokenHandler, std::placeholders::_1, std::placeholders::_2, output),
              std::pair<InitialLinker, PrephaseDefinitions>(
                  [](uint64_t c, std::wstring text, Handler h) { h(c, text); },
                  {
                      {PrephaseToken::Trigraph, L"??" >> chset(L"=/'()!<>-")},
                      {PrephaseToken::RawString, chseq(L"R\"") },
                      {PrephaseToken::PlainCharacter, anychar},
                      {PrephaseToken::EndOfFile, chset(EndOfFile)},
                      {PrephaseToken::StartOfFile, chset(StartOfFile)}
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
                      {FirstPhaseToken::String, chset(String) },
                      {FirstPhaseToken::PlainCharacter, anychar},
                      {FirstPhaseToken::EndOfFile, chset(EndOfFile)},
                      {FirstPhaseToken::StartOfFile, chset(StartOfFile)}
                  }
              ),
              std::pair<FirstToCommentPhaseLinker, CommentPhaseDefinitions>(
                  firstAndCommentPhaseLinker,
                  {
                      {CommentPhaseToken::SingleLineComment, L"//" >> *~chset(L"\n")},
                      {CommentPhaseToken::MultiLineComment, chset(CommentBegin) >> *(~chset(L"*") | (L"*" >> ~chset(L"/"))) >> L"*/"},
                      {CommentPhaseToken::StringLiteral, L"\"" >> stringLiteralContent >> L"\""},
                      {CommentPhaseToken::CharacterLiteral, L"'" >> charLiteralContent >> L"'"},
                      {CommentPhaseToken::String, chset(String) },
                      {CommentPhaseToken::PlainCharacter, anychar},
                      {CommentPhaseToken::EndOfFile, chset(EndOfFile)},
                      {CommentPhaseToken::CommentBegin, chset(CommentBegin)},
                      {CommentPhaseToken::StartOfFile, chset(StartOfFile)}
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
                      {Token::Newline, L"\n" | chset(EndOfFile) | (L"\n" >> chset(EndOfFile))},
                      {Token::HeaderInclude,
                          (L"\n" | startOfFile) >> L"#include" >> *chset(L" \t\v\r\b\f") >>
                              ((L"\"" >> *~chset(L"\"\n") >> L"\"") | (L"<" >> *~chset(L">\n") >> L">") | chset(String))},
                      {Token::PreprocessingOpOrPunc, strset(ops)},
                      {Token::SpecialPreprocessingOpOrPuncSeq, strset({L"<::>", L"<::", L"<:::"})},
                      {Token::NonWhitespaceCharacter, ~usedChar - chset(wchar_t(0xff))},
                      {Token::Ignore, chset(StartOfFile)},
                      {Token::StringLiteral, 
                        (chset(String) | ((chseq(L"u8") | chset(L"uUL")) >> chset(String))) >> !identifier}
                  }
              )
          )
    {
        //for (auto r : identifierNondigitChar.chset.ranges)
            //std::wcerr << std::hex << r.first << L"-" << r.second << std::endl;
    }

    void handle(uint64_t c)
    {
        tokenizerChain.handler(c, {wchar_t(c)});
    }
    void process(uint64_t c)
    {
        if (not started)
        {
            handle(StartOfFile);
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

void PPTokenizer::process(uint64_t c)
{
    impl->process(c);
}

