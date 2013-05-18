#pragma once

#include <iostream>
#include "IPostTokenStream.h"
#include "Helpers.hpp"
#include "StandardData.hpp"

// DebugPostTokenOutputStream: helper class to produce PA2 output format
struct DebugPostTokenOutputStream : IPostTokenStream
{
	void emit_invalid(const std::string& source)
	{
		std::cout << "invalid " << source << std::endl;
	}
	void emit_simple(const std::string& source, ETokenType token_type)
	{
		std::cout << "simple " << source << " " << TokenTypeToStringMap.at(token_type) << std::endl;
	}
	void emit_identifier(const std::string& source)
	{
		std::cout << "identifier " << source << std::endl;
	}
	void emit_literal(const std::string& source, EFundamentalType type, const void* data, size_t nbytes)
	{
		std::cout << "literal " << source << " " << FundamentalTypeToStringMap.at(type) << " " << HexDump(data, nbytes) << std::endl;
	}
	void emit_literal_array(
        const std::string& source, size_t num_elements, EFundamentalType type, const void* data, size_t nbytes)
	{
		std::cout << "literal " << source << " array of " << num_elements << " "
             << FundamentalTypeToStringMap.at(type) << " " << HexDump(data, nbytes) << std::endl;
	}
	void emit_user_defined_literal_character(
        const std::string& source, const std::string& ud_suffix, EFundamentalType type, const void* data, size_t nbytes)
	{
		std::cout << "user-defined-literal " << source << " " << ud_suffix << " character " << FundamentalTypeToStringMap.at(type) << " " << HexDump(data, nbytes) << std::endl;
	}
	void emit_user_defined_literal_string_array(
        const std::string& source, const std::string& ud_suffix, size_t num_elements, EFundamentalType type, const void* data, size_t nbytes)
	{
		std::cout << "user-defined-literal " << source << " " << ud_suffix << " string array of "
             << num_elements << " " << FundamentalTypeToStringMap.at(type) << " " << HexDump(data, nbytes) << std::endl;
	}
	void emit_user_defined_literal_integer(const std::string& source, const std::string& ud_suffix, const std::string& prefix)
	{
		std::cout << "user-defined-literal " << source << " " << ud_suffix << " integer " << prefix << std::endl;
	}
	void emit_user_defined_literal_floating(const std::string& source, const std::string& ud_suffix, const std::string& prefix)
	{
		std::cout << "user-defined-literal " << source << " " << ud_suffix << " floating " << prefix << std::endl;
	}
	void emit_eof()
	{
		std::cout << "eof" << std::endl;
	}
	void emit_new_line()
    {
        //ignoring
    }
};

