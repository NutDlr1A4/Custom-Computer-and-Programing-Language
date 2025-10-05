/*
*	The Generator takes in the tokens created by the lexer, does some analysis of syntax and translates the instructions
*	and identifiers. When it is done, it spits out the long awaited bytes that is the program, ready to be loaded into
*	memory and run!
*/

export module generation;

import std;
import lexer;
import lookahead;
import errorlog;

export class Generator {
public:
	Generator(const std::vector<Token>& tokens, const ErrorLog& logger);

	std::vector<std::uint8_t> Generate();

	bool Good() const;

private:
	void DefineProgramLabels();
	void DefineDataLabels();

private:
	void FlushLine();
	void FlushDataLabel();
	void FlushSection();

private:
	ErrorLog logger;
	Lookahead<Token> tokens;

	std::vector<std::uint8_t> program;

	struct LabelData {
		int line;
		std::size_t offset;
		std::vector<std::uint8_t> data;

		enum class Type {
			PROG,
			DATA
		} type;
	};
	std::map<std::string, LabelData> labels;
};

Generator::Generator(const std::vector<Token>& tokens, const ErrorLog& logger)
	:
	logger("Generator", logger),
	tokens(tokens)
{}

std::vector<std::uint8_t> Generator::Generate() {
	logger.Log("Generating program data...");

	bool prog_found = false;
	while (tokens.At().value().type != TokenType::END_OF_FILE) {
		Token t = tokens.Eat();

		// As the prophecy foretold; The Program is made of sections ...
		if (t.type != TokenType::SECTION) {
			logger.Error("Line is not part of a valid section.", "", t.line);
			FlushLine();
			continue;
		}

		// There must be a newline after section definitions
		if (tokens.At().value().type != TokenType::END_OF_LINE) {
			logger.Error("Excpected newline after section definition.", "", t.line);
			FlushLine();
			continue;
		}

		tokens.Eat();

		// Check valid sections, if none is found raise an error
		if (t.lit == "prog") {
			prog_found = true;
			DefineProgramLabels();
		}
		else if (t.lit == "data") {
			DefineDataLabels();
		}
		else {
			logger.Error(std::string("\"@") + t.lit + "\" is not a valid section.", "", t.line);
			FlushSection();
			continue;
		}
	}

	// There needs to be a program section (obviously)
	if (!prog_found) {
		logger.Error("A program section (@prog) was not found.");
		return {};
	}

	if (Good()) {
		logger.Log("Program generation complete!");
	}

	return {};
}

bool Generator::Good() const {
	return logger.Good();
}

/*
	Goes through each line in the program section and defines labels for each line that has one.
*/
void Generator::DefineProgramLabels() {
	logger.Log("Defining labels...");

	std::size_t offset = 0;
	while (tokens.At().value().type != TokenType::END_OF_FILE) {
		if (tokens.At().value().type == TokenType::SECTION) {
			break;
		}

		Token t = tokens.Eat();

		if (t.type == TokenType::LABEL) {
			// This syntax is kind of arbitrary, might be changed later.
			if (tokens.At().value().type == TokenType::END_OF_LINE) {
				logger.Error("Expected instruction after label definition.", "", t.line);
				FlushLine();
				continue;
			}

			// Check if the label is already defined
			auto label_it = labels.find(t.lit);
			if (label_it != labels.end()) {
				logger.Error(std::string("Redefinition of label \"") + t.lit + "\" (first defined on line " + std::to_string(label_it->second.line) + ").", "", t.line);
				FlushLine();
				continue;
			}

			// Define label
			LabelData label_data{ .line = t.line, .offset = offset, .type = LabelData::Type::PROG };
			labels.insert({ t.lit, label_data });
		}

		// Constant width instructions!
		offset += 4;

		// Discard rest of the line (we don't need that yet)
		FlushLine();
	}

	logger.Log("Labels defined!");
}

/*
	Goes through each label in the data section, then defines, parses and stores the labels and their data.
	If any errors occur the label won't be defined.
*/
void Generator::DefineDataLabels() {
	logger.Log("Parsing data section...");

	std::size_t offset = 0;

	while (tokens.At().value().type != TokenType::END_OF_FILE) {
		if (tokens.At().value().type == TokenType::SECTION) {
			break;
		}

		Token t = tokens.Eat();

		// ... The Data Section is made of labels ...
		if (t.type != TokenType::LABEL) {
			logger.Error("Line is not attached to a data label.", "", t.line);
			FlushLine();
			continue;
		}

		// There must be a newline after data label definitions
		if (tokens.At().value().type != TokenType::END_OF_LINE) {
			logger.Error("Expected newline after data label definiton.", "", t.line);
			FlushLine();
			continue;
		}

		tokens.Eat();

		// Check if the label is already defined
		auto label_it = labels.find(t.lit);
		if (label_it != labels.end()) {
			logger.Error(std::string("Redefinition of label \"") + t.lit + "\" (first defined on line " + std::to_string(label_it->second.line) + ").", "", t.line);
			FlushDataLabel();
			continue;
		}

		LabelData label_data{ .line = t.line, .offset = offset, .type = LabelData::Type::DATA };

		// For each string literal under the label, append it to the stored data for this label
		while (tokens.At().value().type != TokenType::END_OF_FILE) {
			if (tokens.At().value().type == TokenType::LABEL || tokens.At().value().type == TokenType::SECTION) {
				break;
			}

			Token data_token = tokens.Eat();

			// ... A Label is made of string literals ...
			if (data_token.type != TokenType::STR_LIT) {
				logger.Error("Unexpected token.", "", t.line);
				FlushLine();
				continue;
			}

			// There must be a newline after string literals
			if (tokens.At().value().type != TokenType::END_OF_LINE) {
				logger.Error("Excpected newline after string literal.", "", t.line);
				FlushLine();
				continue;
			}

			tokens.Eat();

			// ... And finally, a String Literal is made of characters!
			label_data.data.insert(label_data.data.end(), data_token.lit.begin(), data_token.lit.end());
		}

		// The label might not have contained any string literals, in that case it defines an empty string ("")
		// TODO?: Multiple labels refering to the same content (basically multiple names for the same data)

		// Append a null terminator to the string (important!)
		label_data.data.push_back((std::uint8_t)('\0'));

		// Update offset
		offset += label_data.data.size();

		// Store the label
		labels.insert({ t.lit, label_data });
	}

	logger.Log("Data section parsed!");
}

// TODO: Clean this up please, don't repeat yourself

void Generator::FlushLine() {
	while (tokens.At().value().type != TokenType::END_OF_FILE) {
		Token t = tokens.Eat();
		if (t.type == TokenType::END_OF_LINE) {
			break;
		}
	}
}

void Generator::FlushDataLabel() {
	while (tokens.At().value().type != TokenType::END_OF_FILE) {
		Token t = tokens.Eat();
		if (t.type == TokenType::LABEL || t.type == TokenType::SECTION) {
			break;
		}
	}
}

void Generator::FlushSection() {
	while (tokens.At().value().type != TokenType::END_OF_FILE) {
		Token t = tokens.Eat();
		if (t.type == TokenType::SECTION) {
			break;
		}
	}
}
