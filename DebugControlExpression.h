#pragma once

#include "IControlExpressionEvaluator.h"
#include "IPostTokenStream.h"
#include <string>
#include <iostream>
#include <memory>

struct DebugControlExpressionEvaluatorProxy : IPostTokenStream
{
    std::shared_ptr<IControlExpressionEvaluator> evaluator;

    DebugControlExpressionEvaluatorProxy(std::shared_ptr<IControlExpressionEvaluator> inj)
        : evaluator(inj)
    {}

	void emit_invalid(const std::string& source)
	{
        //ignoring
	}
	void emit_simple(const std::string& source, ETokenType token_type)
	{
        evaluator->processSimple(token_type);
	}
	void emit_identifier(const std::string& source)
	{
        evaluator->processIdentifier(source);
	}
	void emit_literal(const std::string& source, EFundamentalType type, const void* data, size_t nbytes)
	{
        evaluator->processLiteral(type, data, nbytes);
	}
	void emit_literal_array(
        const std::string& source, size_t num_elements, EFundamentalType type, const void* data, size_t nbytes)
	{
        //ignoring
	}
	void emit_user_defined_literal_character(
        const std::string& source, const std::string& ud_suffix, EFundamentalType type, const void* data, size_t nbytes)
	{
        //ignoring
	}
	void emit_user_defined_literal_string_array(
        const std::string& source, const std::string& ud_suffix, size_t num_elements, EFundamentalType type, const void* data, size_t nbytes)
	{
        //ignoring
	}
	void emit_user_defined_literal_integer(const std::string& source, const std::string& ud_suffix, const std::string& prefix)
	{
        //ignoring
	}
	void emit_user_defined_literal_floating(const std::string& source, const std::string& ud_suffix, const std::string& prefix)
	{
        //ignoring
	}
	void emit_eof()
	{
        std::cout << "eof" << std::endl;
	}

    void emit_new_line()
    {
        try
        {
            evaluator->evaluate(
                [](long long int value) { std::cerr << value << std::endl; },
                [](unsigned long long int value) { std::cerr << value << "u" << std::endl; }
            );
        }
        catch(...)
        {
            std::cout << "error" << std::endl;
        }
    }
};

