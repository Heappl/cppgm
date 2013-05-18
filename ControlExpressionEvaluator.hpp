#pragma once

#include <iostream>
#include "IControlExpressionEvaluator.h"
#include "StandardData.hpp"

struct ControlExpressionEvaluator : IControlExpressionEvaluator
{
    ControlExpressionEvaluator();

    void processSimple(ETokenType);
    void processIdentifier(const std::string&);
    void processLiteral(EFundamentalType, const void*, size_t);
    void evaluate(SingedValueHandler, UnsingedValueHandler);
};
