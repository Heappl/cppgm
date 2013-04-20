#include "PostTokenAnalyser.hpp"

PostTokenAnalyser::PostTokenAnalyser(std::shared_ptr<IPostTokenStream> output)
    : output(output)
{
}

void PostTokenAnalyser::emit_whitespace_sequence()
{
}

void PostTokenAnalyser::emit_new_line()
{
}

void PostTokenAnalyser::emit_header_name(const string& data)
{
}

void PostTokenAnalyser::emit_identifier(const string& data)
{
}

void PostTokenAnalyser::emit_pp_number(const string& data)
{
}

void PostTokenAnalyser::emit_character_literal(const string& data)
{
}

void PostTokenAnalyser::emit_user_defined_character_literal(const string& data)
{
}

void PostTokenAnalyser::emit_string_literal(const string& data)
{
}

void PostTokenAnalyser::emit_user_defined_string_literal(const string& data)
{
}

void PostTokenAnalyser::emit_preprocessing_op_or_punc(const string& data)
{
}

void PostTokenAnalyser::emit_non_whitespace_char(const string& data)
{
}

void PostTokenAnalyser::emit_eof()
{
}

