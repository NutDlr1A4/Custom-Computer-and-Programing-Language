/*
* The lexer (also called the tokenizer) has one job, and that is to perform a 'lexical analysis' of the source text,
* which is just converting the source text into more manageable 'tokens'.
* 
* A token is essentialy a group of characters having collective meaning, like a variable name or an integer literal.
* By having to manage a list of tokens with set types instead of a loooooong array of characters, it gets much easier 
* to check for syntax errors and generate code later down the line.
* This might be a little overkill for this tiny, little assembler, but it is also important for compilers!
* 
* Wikipedia: https://en.wikipedia.org/wiki/Lexical_analysis#Lexical_token_and_lexical_tokenization
*/

#pragma once

#include "lookahead.hpp"
#include "errorlog.h"
#include "token.h"

// Returns the escape character made by prepending \ to a given character
// Ex: n -> '\n'
std::optional<char> MakeEscapeCharacter(char c);

// Helper functions for checking number notation in different bases
bool IsHexNotation(const std::string& str);

bool IsBinNotation(const std::string& str);

bool IsDecNotation(const std::string& str);

class Lexer {
public:
	Lexer(const std::string& src, const ErrorLog& logger);

	std::vector<Token> Tokenize();

	bool Good() const;
private:
	void AddToken(TokenType type, std::string_view lit = "", std::uint16_t value = 0);
private:
	ErrorLog logger;

	std::vector<Token> tokens;

	Lookahead<char, std::string> src;
	unsigned int line = 1;
	unsigned int col = 0;
};