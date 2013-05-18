#pragma once

#include "ETokenType.h"
#include "EFundamentalType.h"
#include <string>

typedef void (*SingedValueHandler)(long long int);
typedef void (*UnsingedValueHandler)(unsigned long long int);

struct IControlExpressionEvaluator
{
    virtual void processSimple(ETokenType) = 0;
    virtual void processIdentifier(const std::string&) = 0;
    virtual void processLiteral(EFundamentalType, const void*, size_t) = 0;
    virtual void evaluate(SingedValueHandler, UnsingedValueHandler) = 0;
    virtual ~IControlExpressionEvaluator() {}
};

