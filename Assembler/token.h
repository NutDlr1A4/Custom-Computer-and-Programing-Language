#pragma once

#include <string>

enum class TokenType {
	END_OF_LINE,
	END_OF_FILE,

	LABEL,
	SECTION,

	INT_LIT,
	STR_LIT,
	IDENT
};

struct Token {
	Token() = delete;
	Token(TokenType type, int line, int col, std::string_view lit = "");

	TokenType type;
	std::string lit = "";

	int line = 0;
	int col = 0;
};