#pragma once

#include "IPPTokenStream.h"
#include "IPostTokenStream.h"
#include "StringLiteralsPostTokenProcessor.hpp"
#include <memory>

class PostTokenAnalyser : public IPPTokenStream
{
    std::shared_ptr<IPostTokenStream> output;
    std::shared_ptr<StringLiteralsPostTokenProcessor> stringProcessor;
public:
    PostTokenAnalyser(std::shared_ptr<IPostTokenStream> output);

	virtual void emit_whitespace_sequence();
	virtual void emit_new_line();
	virtual void emit_header_name(const string& data);
	virtual void emit_identifier(const string& data);
	virtual void emit_pp_number(const string& data);
	virtual void emit_character_literal(const string& data);
	virtual void emit_user_defined_character_literal(const string& data);
	virtual void emit_string_literal(const string& data);
	virtual void emit_user_defined_string_literal(const string& data);
	virtual void emit_preprocessing_op_or_punc(const string& data);
	virtual void emit_non_whitespace_char(const string& data);
	virtual void emit_eof();
};

