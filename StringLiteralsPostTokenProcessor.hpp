#pragma once

#include "IPostTokenStream.h"
#include <memory>
#include <string>
#include <vector>

enum class StringCoding
{
    Utf8, Utf16, Utf32
};

class StringLiteralsPostTokenProcessor
{
    std::shared_ptr<IPostTokenStream> output;
    std::vector<std::string> strings;
    std::string source;
    std::string userDefinedSuffix;
    EFundamentalType type;
    bool isInvalid;

public:
    StringLiteralsPostTokenProcessor(std::shared_ptr<IPostTokenStream> output);

    void flush();
    void addString(const std::string&);
    void addUserDefinedString(const std::string&);

private:
    void clear();
};

std::string convertEscapeSeq(std::string arg, StringCoding coding);
unsigned utf8toInt(const std::string& str);
bool checkIfUtf8(const std::string& str);

