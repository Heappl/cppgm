#include "StandardData.hpp"

const std::vector<std::pair<int, int>> AnnexE1_Allowed_RangesSorted =
{
	{0xA8,0xA8},
	{0xAA,0xAA},
	{0xAD,0xAD},
	{0xAF,0xAF},
	{0xB2,0xB5},
	{0xB7,0xBA},
	{0xBC,0xBE},
	{0xC0,0xD6},
	{0xD8,0xF6},
	{0xF8,0xFF},
	{0x100,0x167F},
	{0x1681,0x180D},
	{0x180F,0x1FFF},
	{0x200B,0x200D},
	{0x202A,0x202E},
	{0x203F,0x2040},
	{0x2054,0x2054},
	{0x2060,0x206F},
	{0x2070,0x218F},
	{0x2460,0x24FF},
	{0x2776,0x2793},
	{0x2C00,0x2DFF},
	{0x2E80,0x2FFF},
	{0x3004,0x3007},
	{0x3021,0x302F},
	{0x3031,0x303F},
	{0x3040,0xD7FF},
	{0xF900,0xFD3D},
	{0xFD40,0xFDCF},
	{0xFDF0,0xFE44},
	{0xFE47,0xFFFD},
	{0x10000,0x1FFFD},
	{0x20000,0x2FFFD},
	{0x30000,0x3FFFD},
	{0x40000,0x4FFFD},
	{0x50000,0x5FFFD},
	{0x60000,0x6FFFD},
	{0x70000,0x7FFFD},
	{0x80000,0x8FFFD},
	{0x90000,0x9FFFD},
	{0xA0000,0xAFFFD},
	{0xB0000,0xBFFFD},
	{0xC0000,0xCFFFD},
	{0xD0000,0xDFFFD},
	{0xE0000,0xEFFFD}
};
const std::vector<std::pair<int, int>> AnnexE2_DisallowedInitially_RangesSorted =
{
	{0x300,0x36F},
	{0x1DC0,0x1DFF},
	{0x20D0,0x20FF},
	{0xFE20,0xFE2F}
};
const std::unordered_set<std::string> Digraph_IdentifierLike_Operators =
{
	"new", "delete", "and", "and_eq", "bitand",
	"bitor", "compl", "not", "not_eq", "or",
	"or_eq", "xor", "xor_eq"
};
const std::unordered_set<int> SimpleEscapeSequence_CodePoints =
{
	'\'', '"', '?', '\\', 'a', 'b', 'f', 'n', 'r', 't', 'v'
};

const std::map<EFundamentalType, std::string> FundamentalTypeToStringMap =
{
	{EFundamentalType::FT_SIGNED_CHAR, "signed char"},
	{EFundamentalType::FT_SHORT_INT, "short int"},
	{EFundamentalType::FT_INT, "int"},
	{EFundamentalType::FT_LONG_INT, "long int"},
	{EFundamentalType::FT_LONG_LONG_INT, "long long int"},
	{EFundamentalType::FT_UNSIGNED_CHAR, "unsigned char"},
	{EFundamentalType::FT_UNSIGNED_SHORT_INT, "unsigned short int"},
	{EFundamentalType::FT_UNSIGNED_INT, "unsigned int"},
	{EFundamentalType::FT_UNSIGNED_LONG_INT, "unsigned long int"},
	{EFundamentalType::FT_UNSIGNED_LONG_LONG_INT, "unsigned long long int"},
	{EFundamentalType::FT_WCHAR_T, "wchar_t"},
	{EFundamentalType::FT_CHAR, "char"},
	{EFundamentalType::FT_CHAR16_T, "char16_t"},
	{EFundamentalType::FT_CHAR32_T, "char32_t"},
	{EFundamentalType::FT_BOOL, "bool"},
	{EFundamentalType::FT_FLOAT, "float"},
	{EFundamentalType::FT_DOUBLE, "double"},
	{EFundamentalType::FT_LONG_DOUBLE, "long double"},
	{EFundamentalType::FT_VOID, "void"},
	{EFundamentalType::FT_NULLPTR_T, "nullptr_t"}
};

const std::unordered_map<std::string, ETokenType> StringToTokenTypeMap =
{
	// keywords
	{"alignas", ETokenType::KW_ALIGNAS},
	{"alignof", ETokenType::KW_ALIGNOF},
	{"asm", ETokenType::KW_ASM},
	{"auto", ETokenType::KW_AUTO},
	{"bool", ETokenType::KW_BOOL},
	{"break", ETokenType::KW_BREAK},
	{"case", ETokenType::KW_CASE},
	{"catch", ETokenType::KW_CATCH},
	{"char", ETokenType::KW_CHAR},
	{"char16_t", ETokenType::KW_CHAR16_T},
	{"char32_t", ETokenType::KW_CHAR32_T},
	{"class", ETokenType::KW_CLASS},
	{"const", ETokenType::KW_CONST},
	{"constexpr", ETokenType::KW_CONSTEXPR},
	{"const_cast", ETokenType::KW_CONST_CAST},
	{"continue", ETokenType::KW_CONTINUE},
	{"decltype", ETokenType::KW_DECLTYPE},
	{"default", ETokenType::KW_DEFAULT},
	{"delete", ETokenType::KW_DELETE},
	{"do", ETokenType::KW_DO},
	{"double", ETokenType::KW_DOUBLE},
	{"dynamic_cast", ETokenType::KW_DYNAMIC_CAST},
	{"else", ETokenType::KW_ELSE},
	{"enum", ETokenType::KW_ENUM},
	{"explicit", ETokenType::KW_EXPLICIT},
	{"export", ETokenType::KW_EXPORT},
	{"extern", ETokenType::KW_EXTERN},
	{"false", ETokenType::KW_FALSE},
	{"float", ETokenType::KW_FLOAT},
	{"for", ETokenType::KW_FOR},
	{"friend", ETokenType::KW_FRIEND},
	{"goto", ETokenType::KW_GOTO},
	{"if", ETokenType::KW_IF},
	{"inline", ETokenType::KW_INLINE},
	{"int", ETokenType::KW_INT},
	{"long", ETokenType::KW_LONG},
	{"mutable", ETokenType::KW_MUTABLE},
	{"namespace", ETokenType::KW_NAMESPACE},
	{"new", ETokenType::KW_NEW},
	{"noexcept", ETokenType::KW_NOEXCEPT},
	{"nullptr", ETokenType::KW_NULLPTR},
	{"operator", ETokenType::KW_OPERATOR},
	{"private", ETokenType::KW_PRIVATE},
	{"protected", ETokenType::KW_PROTECTED},
	{"public", ETokenType::KW_PUBLIC},
	{"register", ETokenType::KW_REGISTER},
	{"reinterpret_cast", ETokenType::KW_REINTERPET_CAST},
	{"return", ETokenType::KW_RETURN},
	{"short", ETokenType::KW_SHORT},
	{"signed", ETokenType::KW_SIGNED},
	{"sizeof", ETokenType::KW_SIZEOF},
	{"static", ETokenType::KW_STATIC},
	{"static_assert", ETokenType::KW_STATIC_ASSERT},
	{"static_cast", ETokenType::KW_STATIC_CAST},
	{"struct", ETokenType::KW_STRUCT},
	{"switch", ETokenType::KW_SWITCH},
	{"template", ETokenType::KW_TEMPLATE},
	{"this", ETokenType::KW_THIS},
	{"thread_local", ETokenType::KW_THREAD_LOCAL},
	{"throw", ETokenType::KW_THROW},
	{"true", ETokenType::KW_TRUE},
	{"try", ETokenType::KW_TRY},
	{"typedef", ETokenType::KW_TYPEDEF},
	{"typeid", ETokenType::KW_TYPEID},
	{"typename", ETokenType::KW_TYPENAME},
	{"union", ETokenType::KW_UNION},
	{"unsigned", ETokenType::KW_UNSIGNED},
	{"using", ETokenType::KW_USING},
	{"virtual", ETokenType::KW_VIRTUAL},
	{"void", ETokenType::KW_VOID},
	{"volatile", ETokenType::KW_VOLATILE},
	{"wchar_t", ETokenType::KW_WCHAR_T},
	{"while", ETokenType::KW_WHILE},

	// operators/punctuation
	{"{", ETokenType::OP_LBRACE},
	{"<%", ETokenType::OP_LBRACE},
	{"}", ETokenType::OP_RBRACE},
	{"%>", ETokenType::OP_RBRACE},
	{"[", ETokenType::OP_LSQUARE},
	{"<:", ETokenType::OP_LSQUARE},
	{"]", ETokenType::OP_RSQUARE},
	{":>", ETokenType::OP_RSQUARE},
	{"(", ETokenType::OP_LPAREN},
	{")", ETokenType::OP_RPAREN},
	{"|", ETokenType::OP_BOR},
	{"bitor", ETokenType::OP_BOR},
	{"^", ETokenType::OP_XOR},
	{"xor", ETokenType::OP_XOR},
	{"~", ETokenType::OP_COMPL},
	{"compl", ETokenType::OP_COMPL},
	{"&", ETokenType::OP_AMP},
	{"bitand", ETokenType::OP_AMP},
	{"!", ETokenType::OP_LNOT},
	{"not", ETokenType::OP_LNOT},
	{";", ETokenType::OP_SEMICOLON},
	{":", ETokenType::OP_COLON},
	{"...", ETokenType::OP_DOTS},
	{"?", ETokenType::OP_QMARK},
	{"::", ETokenType::OP_COLON2},
	{".", ETokenType::OP_DOT},
	{".*", ETokenType::OP_DOTSTAR},
	{"+", ETokenType::OP_PLUS},
	{"-", ETokenType::OP_MINUS},
	{"*", ETokenType::OP_STAR},
	{"/", ETokenType::OP_DIV},
	{"%", ETokenType::OP_MOD},
	{"=", ETokenType::OP_ASS},
	{"<", ETokenType::OP_LT},
	{">", ETokenType::OP_GT},
	{"+=", ETokenType::OP_PLUSASS},
	{"-=", ETokenType::OP_MINUSASS},
	{"*=", ETokenType::OP_STARASS},
	{"/=", ETokenType::OP_DIVASS},
	{"%=", ETokenType::OP_MODASS},
	{"^=", ETokenType::OP_XORASS},
	{"xor_eq", ETokenType::OP_XORASS},
	{"&=", ETokenType::OP_BANDASS},
	{"and_eq", ETokenType::OP_BANDASS},
	{"|=", ETokenType::OP_BORASS},
	{"or_eq", ETokenType::OP_BORASS},
	{"<<", ETokenType::OP_LSHIFT},
	{">>", ETokenType::OP_RSHIFT},
	{">>=", ETokenType::OP_RSHIFTASS},
	{"<<=", ETokenType::OP_LSHIFTASS},
	{"==", ETokenType::OP_EQ},
	{"!=", ETokenType::OP_NE},
	{"not_eq", ETokenType::OP_NE},
	{"<=", ETokenType::OP_LE},
	{">=", ETokenType::OP_GE},
	{"&&", ETokenType::OP_LAND},
	{"and", ETokenType::OP_LAND},
	{"||", ETokenType::OP_LOR},
	{"or", ETokenType::OP_LOR},
	{"++", ETokenType::OP_INC},
	{"--", ETokenType::OP_DEC},
	{",", ETokenType::OP_COMMA},
	{"->*", ETokenType::OP_ARROWSTAR},
	{"->", ETokenType::OP_ARROW}
};

const std::map<ETokenType, std::string> TokenTypeToStringMap =
{
	{ETokenType::KW_ALIGNAS, "KW_ALIGNAS"},
	{ETokenType::KW_ALIGNOF, "KW_ALIGNOF"},
	{ETokenType::KW_ASM, "KW_ASM"},
	{ETokenType::KW_AUTO, "KW_AUTO"},
	{ETokenType::KW_BOOL, "KW_BOOL"},
	{ETokenType::KW_BREAK, "KW_BREAK"},
	{ETokenType::KW_CASE, "KW_CASE"},
	{ETokenType::KW_CATCH, "KW_CATCH"},
	{ETokenType::KW_CHAR, "KW_CHAR"},
	{ETokenType::KW_CHAR16_T, "KW_CHAR16_T"},
	{ETokenType::KW_CHAR32_T, "KW_CHAR32_T"},
	{ETokenType::KW_CLASS, "KW_CLASS"},
	{ETokenType::KW_CONST, "KW_CONST"},
	{ETokenType::KW_CONSTEXPR, "KW_CONSTEXPR"},
	{ETokenType::KW_CONST_CAST, "KW_CONST_CAST"},
	{ETokenType::KW_CONTINUE, "KW_CONTINUE"},
	{ETokenType::KW_DECLTYPE, "KW_DECLTYPE"},
	{ETokenType::KW_DEFAULT, "KW_DEFAULT"},
	{ETokenType::KW_DELETE, "KW_DELETE"},
	{ETokenType::KW_DO, "KW_DO"},
	{ETokenType::KW_DOUBLE, "KW_DOUBLE"},
	{ETokenType::KW_DYNAMIC_CAST, "KW_DYNAMIC_CAST"},
	{ETokenType::KW_ELSE, "KW_ELSE"},
	{ETokenType::KW_ENUM, "KW_ENUM"},
	{ETokenType::KW_EXPLICIT, "KW_EXPLICIT"},
	{ETokenType::KW_EXPORT, "KW_EXPORT"},
	{ETokenType::KW_EXTERN, "KW_EXTERN"},
	{ETokenType::KW_FALSE, "KW_FALSE"},
	{ETokenType::KW_FLOAT, "KW_FLOAT"},
	{ETokenType::KW_FOR, "KW_FOR"},
	{ETokenType::KW_FRIEND, "KW_FRIEND"},
	{ETokenType::KW_GOTO, "KW_GOTO"},
	{ETokenType::KW_IF, "KW_IF"},
	{ETokenType::KW_INLINE, "KW_INLINE"},
	{ETokenType::KW_INT, "KW_INT"},
	{ETokenType::KW_LONG, "KW_LONG"},
	{ETokenType::KW_MUTABLE, "KW_MUTABLE"},
	{ETokenType::KW_NAMESPACE, "KW_NAMESPACE"},
	{ETokenType::KW_NEW, "KW_NEW"},
	{ETokenType::KW_NOEXCEPT, "KW_NOEXCEPT"},
	{ETokenType::KW_NULLPTR, "KW_NULLPTR"},
	{ETokenType::KW_OPERATOR, "KW_OPERATOR"},
	{ETokenType::KW_PRIVATE, "KW_PRIVATE"},
	{ETokenType::KW_PROTECTED, "KW_PROTECTED"},
	{ETokenType::KW_PUBLIC, "KW_PUBLIC"},
	{ETokenType::KW_REGISTER, "KW_REGISTER"},
	{ETokenType::KW_REINTERPET_CAST, "KW_REINTERPET_CAST"},
	{ETokenType::KW_RETURN, "KW_RETURN"},
	{ETokenType::KW_SHORT, "KW_SHORT"},
	{ETokenType::KW_SIGNED, "KW_SIGNED"},
	{ETokenType::KW_SIZEOF, "KW_SIZEOF"},
	{ETokenType::KW_STATIC, "KW_STATIC"},
	{ETokenType::KW_STATIC_ASSERT, "KW_STATIC_ASSERT"},
	{ETokenType::KW_STATIC_CAST, "KW_STATIC_CAST"},
	{ETokenType::KW_STRUCT, "KW_STRUCT"},
	{ETokenType::KW_SWITCH, "KW_SWITCH"},
	{ETokenType::KW_TEMPLATE, "KW_TEMPLATE"},
	{ETokenType::KW_THIS, "KW_THIS"},
	{ETokenType::KW_THREAD_LOCAL, "KW_THREAD_LOCAL"},
	{ETokenType::KW_THROW, "KW_THROW"},
	{ETokenType::KW_TRUE, "KW_TRUE"},
	{ETokenType::KW_TRY, "KW_TRY"},
	{ETokenType::KW_TYPEDEF, "KW_TYPEDEF"},
	{ETokenType::KW_TYPEID, "KW_TYPEID"},
	{ETokenType::KW_TYPENAME, "KW_TYPENAME"},
	{ETokenType::KW_UNION, "KW_UNION"},
	{ETokenType::KW_UNSIGNED, "KW_UNSIGNED"},
	{ETokenType::KW_USING, "KW_USING"},
	{ETokenType::KW_VIRTUAL, "KW_VIRTUAL"},
	{ETokenType::KW_VOID, "KW_VOID"},
	{ETokenType::KW_VOLATILE, "KW_VOLATILE"},
	{ETokenType::KW_WCHAR_T, "KW_WCHAR_T"},
	{ETokenType::KW_WHILE, "KW_WHILE"},
	{ETokenType::OP_LBRACE, "OP_LBRACE"},
	{ETokenType::OP_RBRACE, "OP_RBRACE"},
	{ETokenType::OP_LSQUARE, "OP_LSQUARE"},
	{ETokenType::OP_RSQUARE, "OP_RSQUARE"},
	{ETokenType::OP_LPAREN, "OP_LPAREN"},
	{ETokenType::OP_RPAREN, "OP_RPAREN"},
	{ETokenType::OP_BOR, "OP_BOR"},
	{ETokenType::OP_XOR, "OP_XOR"},
	{ETokenType::OP_COMPL, "OP_COMPL"},
	{ETokenType::OP_AMP, "OP_AMP"},
	{ETokenType::OP_LNOT, "OP_LNOT"},
	{ETokenType::OP_SEMICOLON, "OP_SEMICOLON"},
	{ETokenType::OP_COLON, "OP_COLON"},
	{ETokenType::OP_DOTS, "OP_DOTS"},
	{ETokenType::OP_QMARK, "OP_QMARK"},
	{ETokenType::OP_COLON2, "OP_COLON2"},
	{ETokenType::OP_DOT, "OP_DOT"},
	{ETokenType::OP_DOTSTAR, "OP_DOTSTAR"},
	{ETokenType::OP_PLUS, "OP_PLUS"},
	{ETokenType::OP_MINUS, "OP_MINUS"},
	{ETokenType::OP_STAR, "OP_STAR"},
	{ETokenType::OP_DIV, "OP_DIV"},
	{ETokenType::OP_MOD, "OP_MOD"},
	{ETokenType::OP_ASS, "OP_ASS"},
	{ETokenType::OP_LT, "OP_LT"},
	{ETokenType::OP_GT, "OP_GT"},
	{ETokenType::OP_PLUSASS, "OP_PLUSASS"},
	{ETokenType::OP_MINUSASS, "OP_MINUSASS"},
	{ETokenType::OP_STARASS, "OP_STARASS"},
	{ETokenType::OP_DIVASS, "OP_DIVASS"},
	{ETokenType::OP_MODASS, "OP_MODASS"},
	{ETokenType::OP_XORASS, "OP_XORASS"},
	{ETokenType::OP_BANDASS, "OP_BANDASS"},
	{ETokenType::OP_BORASS, "OP_BORASS"},
	{ETokenType::OP_LSHIFT, "OP_LSHIFT"},
	{ETokenType::OP_RSHIFT, "OP_RSHIFT"},
	{ETokenType::OP_RSHIFTASS, "OP_RSHIFTASS"},
	{ETokenType::OP_LSHIFTASS, "OP_LSHIFTASS"},
	{ETokenType::OP_EQ, "OP_EQ"},
	{ETokenType::OP_NE, "OP_NE"},
	{ETokenType::OP_LE, "OP_LE"},
	{ETokenType::OP_GE, "OP_GE"},
	{ETokenType::OP_LAND, "OP_LAND"},
	{ETokenType::OP_LOR, "OP_LOR"},
	{ETokenType::OP_INC, "OP_INC"},
	{ETokenType::OP_DEC, "OP_DEC"},
	{ETokenType::OP_COMMA, "OP_COMMA"},
	{ETokenType::OP_ARROWSTAR, "OP_ARROWSTAR"},
	{ETokenType::OP_ARROW, "OP_ARROW"}
};

