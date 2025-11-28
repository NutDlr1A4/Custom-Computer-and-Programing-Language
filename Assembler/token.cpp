#include "token.h"

Token::Token(TokenType type, int line, int col, std::string_view lit, std::uint16_t value)
	:
	type(type),
	lit(lit),
	line(line),
	col(col),
	value(value)
{}