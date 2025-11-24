#include "token.h"

Token::Token(TokenType type, int line, int col, std::string_view lit)
	:
	type(type),
	lit(lit),
	line(line),
	col(col) 
{}