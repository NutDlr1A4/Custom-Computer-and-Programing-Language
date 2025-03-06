export module lexer;

import std;

// Returns the escape character made by prepending \ to a given character
// Ex: n -> '\n'
std::optional<char> MakeEscapeCharacter(char c);

// Helper functions for checking number notation in different bases
bool IsHexNotation(const std::string& str);
bool IsBinNotation(const std::string& str);
bool IsDecNotation(const std::string& str);

enum class TokenType {
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
	Lexer(const std::string& src, bool doLogging = false);

	std::vector<Token> Tokenize();

	bool Good() const;
private:
	std::optional<char> At(size_t offset = 0) const;
	char Eat();
	void AddToken(TokenType type, std::string_view lit = "");

	void Log(std::string_view msg) const;
	void Error(std::string_view msg, std::string_view desc = "", bool fatal = true);

private:
	bool logging;
	bool good = true;

	const std::string& src;
	std::vector<Token> tokens;

	size_t index = 0;
	unsigned int line = 1;
	unsigned int col = 0;
};

Lexer::Lexer(const std::string& src, bool doLogging)
	:
	logging(doLogging),
	src(src) {
}

std::vector<Token> Lexer::Tokenize() {
	Log("Tokenizing source file...");

	std::string buf;

	while (At().has_value()) {
		buf.clear();

		// Empty line
		if (std::isspace(At().value())) {
			// If there is a newline, add a token and increment line
			if (At().value() == '\n') {
				AddToken(TokenType::END_OF_LINE);
				col = 0;
				line++;
			}
			Eat();
		}
		// Comments
		else if (At().value() == ';') {
			// Eat characters until end of line, effectively flushing it
			// TODO: Add multi-line comments, using /* ... */ perhaps?
			while (At().has_value() && At().value() != '\n') {
				Eat();
			}
		}
		// Labels
		else if (At().value() == '.') {
			// Eat the '.' and fill buffer
			Eat();
			while (At().has_value() && std::isalnum(At().value())) {
				buf.push_back(Eat());
			}

			if (buf.empty()) {
				// Ex: .
				Error("Empty label definition.");
				continue;
			}

			if (std::isdigit(buf.front())) {
				// Ex: .2label
				Error("Invalid label defintion \"" + buf + "\".", "Label name can't begin with a digit.");
				continue;
			}

			// Add token
			AddToken(TokenType::LABEL, buf);
		}
		// Section
		else if (At().value() == '@') {
			// Eat the '@' and fill buffer
			Eat();
			while (At().has_value() && std::isalnum(At().value())) {
				buf.push_back(Eat());
			}

			if (buf.empty()) {
				// Ex: @
				Error("Empty section definiton.");
				continue;
			}

			if (std::isdigit(buf.front())) {
				// Ex: @2section
				Error("Invalid section defintion \"" + buf + "\".", "Section name can't begin with a digit.");
				continue;
			}

			// Add token
			AddToken(TokenType::SECTION, buf);
		}
		// String/character literal
		// Code for each case was so similar I combined them into one section.
		// Both cases differs in a few lines, so I opted for if-statements instead.
		else if (At().value() == '"' || At().value() == '\'') {
			// Store what kind of quote is used (" for strings, ' for characters)
			char quoteType = Eat();

			// Fill buffer until the literal is closed
			// Might be a waste to read so much from a character literal, but if the literal is
			// valid (which it should be to begin with) it shouldn't be a problem.
			while (At().has_value() && At().value() != quoteType) {
				// If there's a newline before the literal is closed, it's invalid
				if (At().value() == '\n') {
					if (quoteType == '"') {
						// Ex: "abc
						Error("String literal missing terminating quote.");
					}
					else {
						// Ex: 'a
						Error("Character literal missing terminating quote.");
					}
					break;
				}

				char c = Eat();

				// If the character is a '\', the next character is an escape-character
				// Note: If the next character is not a valid escape-character, it is 
				// interpreted as is. For example: '\p' just becomes 'p'
				// A warning will be printed to the screen.
				if (c == '\\' && At().has_value()) {
					std::optional<char> escapeChar = MakeEscapeCharacter(At().value());

					if (escapeChar.has_value()) {
						buf.push_back(escapeChar.value());
					}
					else {
						// Ex: '\p'
						Error(std::string("Unrecognized escape character \'\\") + At().value() + "\'.", "", false);
						buf.push_back(At().value());
					}
					Eat();
				}
				else {
					buf.push_back(c);
				}
			}

			// Eat the terminating quote (if it exists)
			if (At().value() == quoteType)
				Eat();

			// Add the token. 
			// If it's a character literal literal a couple of checks must be made 
			// to check if it's valid.
			if (quoteType == '"') {
				AddToken(TokenType::STR_LIT, buf);
			}
			else {
				// Ex: ''
				if (buf.length() == 0) {
					Error("Empty character literal.");
					continue;
				}

				// Ex: 'abc'
				if (buf.length() > 1) {
					Error("Character literal contains more than one character.", "Use double quotes(\") to define string literals.");
					continue;
				}

				// Add token
				AddToken(TokenType::INT_LIT, buf);
			}
		}
		// Integer literal
		// TODO: More extensive test.
		// I am not 100% this gives correct results, for example during overflows or underflows
		else if (std::isdigit(At().value()) || At().value() == '-') {
			// Fill buffer
			buf.push_back(Eat());
			while (At().has_value() && std::isalnum(At().value())) {
				buf.push_back(Eat());
			}

			// Check and store if the number is negative
			bool negative = false;
			if (buf.at(0) == '-') {
				if (buf.size() == 1) {
					Error("Empty integer literal after \'-\'.");
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
				Error("Invalid integer literal format \"" + buf + "\".", "Only decimal, binary (0b) and hexadecimal (0x) are supported.");
				continue;
			}

			// Negate the value if it's negative, and
			// check if the value is too large/small for 16 bits
			std::int32_t value = std::stoi(buf, nullptr, base);
			if (negative) {
				value = -value;
				if (value < std::numeric_limits<std::int16_t>::min())
					Error("Integer literal \"-" + buf + "\" too small to be represented with 16 bits.", "", false);
				if (value > std::numeric_limits<std::int16_t>::max())
					Error("Integer literal \"-" + buf + "\" too large to be represented with 16 bits.", "", false);
			}
			else {
				if (value > std::numeric_limits<std::uint16_t>::max())
					Error("Integer literal \"" + buf + "\" too large to be represented with 16 bits.", "", false);
			}

			// Cast value to 16-bits and add the token
			AddToken(TokenType::INT_LIT, std::to_string((std::uint16_t)value));
		}
		// Identifiers and instructions
		// Distinction between registers, instructions and identifiers happen during generation
		// For now, all are handled as identifiers
		else if (std::isalpha(At().value())) {
			// Fill buffer
			while (At().has_value() && std::isalnum(At().value())) {
				buf.push_back(Eat());
			}

			// Add token
			AddToken(TokenType::IDENT, buf);
		}
		// Unknown
		else {
			Error(std::string("Invalid or unknown symbol \'") + At().value() + "\'.");
			Eat();
		}
	}

	AddToken(TokenType::END_OF_FILE);

	if (Good())
		Log("Tokenization complete!");

	return tokens;
}

bool Lexer::Good() const {
	return good;
}

std::optional<char> Lexer::At(size_t offset) const {
	if (index + offset >= src.length()) {
		return {};
	}

	return src.at(index + offset);
}

char Lexer::Eat() {
	char c = src.at(index);
	index++;
	col++;
	return c;
}

void Lexer::AddToken(TokenType type, std::string_view lit) {
	Token t(type, line, col, lit);
	tokens.push_back(t);
}

void Lexer::Log(std::string_view msg) const {
	if (logging)
		std::println("[Lexer] {}", msg);
}

void Lexer::Error(std::string_view msg, std::string_view desc, bool fatal) {
	if (logging) {
		if (fatal) {
			std::println("[Lexer] Error   (line {}): {}", line, msg);
		}
		else {
			std::println("[Lexer] Warning (line {}): {}", line, msg);
		}
		if (!desc.empty()) std::println("\t{}", desc);
	}

	if (fatal)
		good = false;
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