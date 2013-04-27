#include "PostTokenAnalyser.hpp"
#include "StandardData.hpp"
#include "Helpers.hpp"
#include <vector>

PostTokenAnalyser::PostTokenAnalyser(std::shared_ptr<IPostTokenStream> output)
    : output(output)
{
}

void PostTokenAnalyser::emit_whitespace_sequence()
{
    //ignoring
}

void PostTokenAnalyser::emit_new_line()
{
    //ignoring
}

void PostTokenAnalyser::emit_header_name(const string& data)
{
}

void PostTokenAnalyser::emit_identifier(const string& data)
{
    auto it = StringToTokenTypeMap.find(data);
    if (it != StringToTokenTypeMap.end())
        output->emit_simple(data, it->second);
    else
        output->emit_identifier(data);
}

void PostTokenAnalyser::emit_pp_number(const string& data)
{
    if ((data.find('.') == std::string::npos)
         && (data.find('e') == std::string::npos)
         && (data.find('E') == std::string::npos))
    {
        int aux = int(PA2Decode_double(data));
        output->emit_literal(data, EFundamentalType::FT_INT, &aux, sizeof(aux));
    }
    else
    {
        double aux = PA2Decode_double(data);
        output->emit_literal(data, EFundamentalType::FT_DOUBLE, &aux, sizeof(aux));
    }
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
    auto it = StringToTokenTypeMap.find(data);
    if (it != StringToTokenTypeMap.end())
        output->emit_simple(data, it->second);
    else
        output->emit_invalid(data);
}

void PostTokenAnalyser::emit_non_whitespace_char(const string& data)
{
    output->emit_invalid(data);
}

void PostTokenAnalyser::emit_eof()
{
    output->emit_eof();
}

