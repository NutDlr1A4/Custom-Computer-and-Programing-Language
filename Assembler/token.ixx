export module token;

import std;

export enum class TokenType {
	END_OF_LINE,
	END_OF_FILE,

	LABEL,
	SECTION,

	INT_LIT,
	STR_LIT,
	IDENT
};

export struct Token {
	Token() = delete;
	Token(TokenType type, int line, int col, std::string_view lit = "")
		:
		type(type),
		lit(lit),
		line(line),
		col(col) {
	}

	TokenType type;
	std::string lit = "";

	int line = 0;
	int col = 0;
};