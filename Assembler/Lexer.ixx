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

export module lexer;

import std;
import lookahead;
import errorlog;

// Returns the escape character made by prepending \ to a given character
// Ex: n -> '\n'
std::optional<char> MakeEscapeCharacter(char c);

// Helper functions for checking number notation in different bases
bool IsHexNotation(const std::string& str);
bool IsBinNotation(const std::string& str);
bool IsDecNotation(const std::string& str);

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

export class Lexer {
public:
	Lexer(const std::string& src, const ErrorLog& logger);

	std::vector<Token> Tokenize();

	bool Good() const;
private:
	void AddToken(TokenType type, std::string_view lit = "");
private:
	ErrorLog logger;

	std::vector<Token> tokens;

	Lookahead<char, std::string> src;
	unsigned int line = 1;
	unsigned int col = 0;
};

Lexer::Lexer(const std::string& src, const ErrorLog& logger)
	:
	logger("Lexer", logger),
	src(src)
{}

std::vector<Token> Lexer::Tokenize() {
	logger.Log("Tokenizing source file...");

	std::string buf;

	while (src.At().has_value()) {
		buf.clear();

		// Empty line
		if (std::isspace(src.At().value())) {
			// If there is a newline, add a token and increment line
			if (src.At().value() == '\n') {
				if (tokens.size() != 0 && tokens.back().type != TokenType::END_OF_LINE) {
					AddToken(TokenType::END_OF_LINE);
				}
				col = 0;
				line++;
			}
			src.Eat();
		}
		// Comments
		else if (src.At().value() == ';') {
			// Eat characters until end of line, effectively flushing it
			// TODO: Add multi-line comments, using /* ... */ perhaps?
			while (src.At().has_value() && src.At().value() != '\n') {
				src.Eat();
			}
		}
		// Labels
		else if (src.At().value() == '.') {
			// Eat the '.' and fill buffer
			src.Eat();
			while (src.At().has_value() && std::isalnum(src.At().value())) {
				buf.push_back(src.Eat());
			}

			if (buf.empty()) {
				// Ex: .
				logger.Error("Empty label definition.", "", line);
				continue;
			}

			if (std::isdigit(buf.front())) {
				// Ex: .2label
				logger.Error("Invalid label defintion \"" + buf + "\".", "Label name can't begin with a digit.", line);
				continue;
			}

			// Add token
			AddToken(TokenType::LABEL, buf);
		}
		// Section
		else if (src.At().value() == '@') {
			// Eat the '@' and fill buffer
			src.Eat();
			while (src.At().has_value() && std::isalnum(src.At().value())) {
				buf.push_back(src.Eat());
			}

			if (buf.empty()) {
				// Ex: @
				logger.Error("Empty section definiton.", "", line);
				continue;
			}

			if (std::isdigit(buf.front())) {
				// Ex: @2section
				logger.Error("Invalid section defintion \"" + buf + "\".", "Section name can't begin with a digit.", line);
				continue;
			}

			// Add token
			AddToken(TokenType::SECTION, buf);
		}
		// String/character literal
		// Code for each case was so similar I combined them into one section.
		// Both cases differs in a few lines, so I opted for if-statements instead.
		else if (src.At().value() == '"' || src.At().value() == '\'') {
			// Store what kind of quote is used (" for strings, ' for characters)
			char quoteType = src.Eat();

			// Fill buffer until the literal is closed
			// Might be a waste to read so much from a character literal, but if the literal is
			// valid (which it should be to begin with) it shouldn't be a problem.
			while (src.At().has_value() && src.At().value() != quoteType) {
				// If there's a newline before the literal is closed, it's invalid
				if (src.At().value() == '\n') {
					if (quoteType == '"') {
						// Ex: "abc
						logger.Error("String literal missing terminating quote.", "", line);
					}
					else {
						// Ex: 'a
						logger.Error("Character literal missing terminating quote.", "", line);
					}
					break;
				}

				char c = src.Eat();

				// If the character is a '\', the next character is an escape-character
				// Note: If the next character is not a valid escape-character, it is 
				// interpreted as is. For example: '\p' just becomes 'p'
				// A warning will be printed to the screen.
				if (c == '\\' && src.At().has_value()) {
					std::optional<char> escapeChar = MakeEscapeCharacter(src.At().value());

					if (escapeChar.has_value()) {
						buf.push_back(escapeChar.value());
					}
					else {
						// Ex: '\p'
						logger.Warning(std::string("Unrecognized escape character \'\\") + src.At().value() + "\'.", 
									   std::string("Interpreted as \'") + src.At().value() + "\'",
									   line);
						buf.push_back(src.At().value());
					}
					src.Eat();
				}
				else {
					buf.push_back(c);
				}
			}

			// Eat the terminating quote (if it exists)
			if (src.At().value() == quoteType)
				src.Eat();

			// Add the token. 
			// If it's a character literal literal a couple of checks must be made 
			// to check if it's valid.
			if (quoteType == '"') {
				AddToken(TokenType::STR_LIT, buf);
			}
			else {
				// Ex: ''
				if (buf.length() == 0) {
					logger.Error("Empty character literal.", "", line);
					continue;
				}

				// Ex: 'abc'
				if (buf.length() > 1) {
					logger.Error("Character literal contains more than one character.", "Use double quotes(\") to define string literals.", line);
					continue;
				}

				// Add token
				AddToken(TokenType::INT_LIT, buf);
			}
		}
		// Integer literal
		// TODO: More extensive test.
		// I am not 100% this gives correct results, for example during overflows or underflows
		else if (std::isdigit(src.At().value()) || src.At().value() == '-') {
			// Fill buffer
			buf.push_back(src.Eat());
			while (src.At().has_value() && std::isalnum(src.At().value())) {
				buf.push_back(src.Eat());
			}

			// Check and store if the number is negative
			bool negative = false;
			if (buf.at(0) == '-') {
				if (buf.size() == 1) {
					logger.Error("Empty integer literal after \'-\'.", "", line);
					continue;
				}

				negative = true;
				buf = buf.substr(1);
			}

			// Find the base of the number
			int base = 10;
			if (IsHexNotation(buf)) {
				base = 16;
				buf = buf.substr(2);
			}
			else if (IsBinNotation(buf)) {
				base = 2;
				buf = buf.substr(2);
			}
			else if (!IsDecNotation(buf)) {
				logger.Error("Invalid integer literal format \"" + buf + "\".", "Only decimal, binary (0b) and hexadecimal (0x) are supported.", line);
				continue;
			}

			// Negate the value if it's negative, and
			// check if the value is too large/small for 16 bits
			std::int32_t value = std::stoi(buf, nullptr, base);
			if (negative) {
				value = -value;
				if (value < std::numeric_limits<std::int16_t>::min())
					logger.Warning("Integer literal \"-" + buf + "\" too small to be represented with 16 bits.", "", line);
				if (value > std::numeric_limits<std::int16_t>::max())
					logger.Warning("Integer literal \"-" + buf + "\" too large to be represented with 16 bits.", "", line);
			}
			else {
				if (value > std::numeric_limits<std::uint16_t>::max())
					logger.Warning("Integer literal \"" + buf + "\" too large to be represented with 16 bits.", "", line);
			}

			// Cast value to 16-bits and add the token
			AddToken(TokenType::INT_LIT, std::to_string((std::uint16_t)value));
		}
		// Identifiers and instructions
		// Distinction between registers, instructions and identifiers happen during generation
		// For now, all are handled as identifiers
		else if (std::isalpha(src.At().value())) {
			// Fill buffer
			while (src.At().has_value() && std::isalnum(src.At().value())) {
				buf.push_back(src.Eat());
			}

			// Add token
			AddToken(TokenType::IDENT, buf);
		}
		// Unknown
		else {
			logger.Error(std::string("Invalid or unknown symbol \'") + src.At().value() + "\'.", "", line);
			src.Eat();
		}
	}

	AddToken(TokenType::END_OF_FILE);

	if (Good())
		logger.Log("Tokenization complete!");

	return tokens;
}

bool Lexer::Good() const {
	return logger.Good();
}

void Lexer::AddToken(TokenType type, std::string_view lit) {
	Token t(type, line, col, lit);
	tokens.push_back(t);
}

std::optional<char> MakeEscapeCharacter(char c) {
	switch (c) {
	case '\'':
	case '\"':
	case '\\':
		return c;
	case 't':
		return '\t';
	case 'n':
		return '\n';
	case '0':
		return '\0';
	default:
		return std::nullopt;
	}
}

bool IsHexNotation(const std::string& str) {
	return str.compare(0, 2, "0x") == 0
		&& str.size() > 2
		&& str.find_first_not_of("0123456789abcdefABCDEF", 2) == std::string::npos;
}

bool IsBinNotation(const std::string& str) {
	return str.compare(0, 2, "0b") == 0
		&& str.size() > 2
		&& str.find_first_not_of("01", 2) == std::string::npos;
}

bool IsDecNotation(const std::string& str) {
	return str.find_first_not_of("0123456789") == std::string::npos;
}
