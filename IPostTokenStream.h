#pragma once

#include "ETokenType.h"
#include "EFundamentalType.h"

struct IPostTokenStream
{
	// output: invalid <source>
	virtual void emit_invalid(const std::string& source) = 0;
	
	// output: simple <source> <token_type>
	virtual void emit_simple(const std::string& source, ETokenType token_type) = 0;

	// output: identifier <source>
	virtual void emit_identifier(const std::string& source) = 0;
	
	// output: literal <source> <type> <hexdump(data,nbytes)>
	virtual void emit_literal(
        const std::string& source, EFundamentalType type, const void* data, size_t nbytes) = 0;
	
	// output: literal <source> array of <num_elements> <type> <hexdump(data,nbytes)>
	virtual void emit_literal_array(
        const std::string& source, size_t num_elements, EFundamentalType type, const void* data, size_t nbytes) = 0;
	

	// output: user-defined-literal <source> <ud_suffix> character <type> <hexdump(data,nbytes)>
	virtual void emit_user_defined_literal_character(
        const std::string& source, const std::string& ud_suffix, EFundamentalType type, const void* data, size_t nbytes) = 0;
	

	// output: user-defined-literal <source> <ud_suffix> string array of <num_elements> <type> <hexdump(data, nbytes)>
	virtual void emit_user_defined_literal_string_array(
        const std::string& source,
        const std::string& ud_suffix,
        size_t num_elements,
        EFundamentalType type,
        const void* data,
        size_t nbytes) = 0;
	

	// output: user-defined-literal <source> <ud_suffix> <prefix>
	virtual void emit_user_defined_literal_integer(
        const std::string& source, const std::string& ud_suffix, const std::string& prefix) = 0;
	

	// output: user-defined-literal <source> <ud_suffix> <prefix>
	virtual void emit_user_defined_literal_floating(
        const std::string& source, const std::string& ud_suffix, const std::string& prefix) = 0;
	
	// output : eof
	virtual void emit_eof() = 0;
};

