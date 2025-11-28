#include "generation.h"

Generator::Generator(const std::vector<Token>& tokens, const ErrorLog& logger)
	:
	logger("Generator", logger),
	tokens(tokens)
{}

std::vector<std::uint8_t> Generator::Generate() {
	logger.Log("Generating program data...");

	bool prog_found = false;
	std::size_t prog_index = 0;
	std::vector<std::string> checked_sections;
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

		// Check if section is valid
		if (t.lit != "prog" && t.lit != "data") {
			logger.Error(std::string("\"@") + t.lit + "\" is not a valid section.", "", t.line);
			if (tokens.At().value().type != TokenType::SECTION) FlushSection();
			continue;
		}

		// Check if the section already has been checked
		if (std::find(checked_sections.begin(), checked_sections.end(), t.lit) != checked_sections.end()) {
			logger.Error(std::string("Duplicate section \"@") + t.lit + "\"", "", t.line);
			FlushSection();
			continue;
		}

		checked_sections.push_back(t.lit);

		if (t.lit == "prog") {
			prog_found = true;
			prog_index = tokens.Index();
			DefineProgramLabels();
		}
		else {
			DefineDataLabels();
		}
	}

	// There needs to be a program section (obviously)
	if (!prog_found) {
		logger.Error("A program section (@prog) was not found.");
	}

	if (!Good()) {
		return {};
	}

	tokens.Seek(prog_index);
	ParseProgramSection();

	if (!Good()) {
		return {};
	}

	program.insert(program.end(), data_size, 0);

	for (auto& l : labels) {
		if (l.second.type == LabelData::Type::PROG) continue;

		std::copy(l.second.data.begin(), l.second.data.end(), program.begin() + prog_size + l.second.offset);
	}

	return program;
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
			if (tokens.At().value().type != TokenType::INSTRUCTION) {
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

	prog_size = offset;

	if (Good()) logger.Log("Labels defined!");
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

		Token label_token = tokens.Eat();

		// ... The Data Section is made of labels ...
		if (label_token.type != TokenType::LABEL) {
			logger.Error("Line is not attached to a data label.", "", label_token.line);
			FlushLine();
			continue;
		}

		// There must be a newline after data label definitions
		if (tokens.At().value().type != TokenType::END_OF_LINE) {
			logger.Error("Expected newline after data label definiton.", "", label_token.line);
			FlushLine();
			continue;
		}

		tokens.Eat();

		// Check if the label is already defined
		auto label_it = labels.find(label_token.lit);
		if (label_it != labels.end()) {
			logger.Error(std::string("Redefinition of label \"") + label_token.lit + "\" (first defined on line " + std::to_string(label_it->second.line) + ").", "", label_token.line);
			FlushDataLabel();
			continue;
		}

		LabelData label_data{ .line = label_token.line, .offset = offset, .type = LabelData::Type::DATA };

		// For each string literal under the label, append it to the stored data for this label
		while (tokens.At().value().type != TokenType::END_OF_FILE) {
			if (tokens.At().value().type == TokenType::LABEL || tokens.At().value().type == TokenType::SECTION) {
				break;
			}

			Token data_token = tokens.Eat();

			// ... A Label is made of string literals ...
			if (data_token.type != TokenType::STR_LIT) {
				logger.Error("Unexpected token, expected string literal.", "", data_token.line);
				FlushLine();
				continue;
			}

			// There must be a newline after string literals
			if (tokens.At().value().type != TokenType::END_OF_LINE) {
				logger.Error("Excpected newline after string literal.", "", data_token.line);
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
		labels.insert({ label_token.lit, label_data });
	}

	data_size = offset;

	if (Good()) logger.Log("Data section parsed!");
}

void Generator::ParseProgramSection() {
	logger.Log("Parsing instructions...");

	while (tokens.At().value().type != TokenType::END_OF_FILE) {
		Token instruction_token = tokens.At().value();

		if (instruction_token.type == TokenType::SECTION) {
			break;
		}

		// A line might start with a label definition, just discard it if that's the case
		if (instruction_token.type == TokenType::LABEL) {
			tokens.Eat();
			instruction_token = tokens.At().value();
		}

		// If the line doesn't start with an instruction, someone, somewhere has fucked up 
		if (instruction_token.type != TokenType::INSTRUCTION) {
			logger.Error("Line is not an instruction.", "", instruction_token.line);
			FlushLine();
			continue;
		}

		tokens.Eat();

		std::vector<std::uint8_t> register_values;
		std::vector<std::uint16_t> integer_literal_values;
		std::string signature;

		while (tokens.At().value().type != TokenType::END_OF_FILE) {
			Token operand_token = tokens.Eat();
			
			if (operand_token.type == TokenType::END_OF_LINE) {
				break;
			}

			// Fill out signature and literal values
			if (operand_token.type == TokenType::REGISTER) {
				signature.push_back('r');

				// We know the register exists because of the lexer
				auto reg_it = valid_register_map.find(operand_token.lit);
				register_values.push_back(reg_it->second);
			}
			else if (operand_token.type == TokenType::INT_LIT) {
				signature.push_back('i');
				integer_literal_values.push_back(operand_token.value);
			}
			else if (operand_token.type == TokenType::IDENT) {
				signature.push_back('i');

				auto label_it = labels.find(operand_token.lit);
				if (label_it == labels.end()) {
					logger.Error("Undefined identifier \"" + operand_token.lit + "\"", "", operand_token.line);
					continue;
				}

				// Data labels are offset from the program by the program size
				if (label_it->second.type == LabelData::Type::PROG) {
					integer_literal_values.push_back((std::uint16_t)label_it->second.offset);
				}
				else {
					integer_literal_values.push_back((std::uint16_t)(prog_size + label_it->second.offset));
				}
			}
			else {
				logger.Error("Unexpected token", "", operand_token.line);
				FlushLine();
				continue;
			}
		}

		// We know the instruction exists because of the lexer
		auto inst_it = valid_instruction_map.find(instruction_token.lit);

		// Find the instruction with the correct signature (if it exists)
		auto variant_it = std::find_if(
			inst_it->second.begin(), 
			inst_it->second.end(), 
			[signature](const InstructionVariant& var)
			{
				return var.signature == signature;
			}
		);
		if (variant_it == inst_it->second.end()) {
			logger.Error("Invalid signature for instruction \"" + instruction_token.lit + "\"", "", instruction_token.line);
			continue;
		}

		// Now, append the bytes and we're done!
		program.push_back(variant_it->opcode);
		program.insert(program.end(), 3, 0);
		
		if (!signature.empty() && signature != "i") {
			program[program.size() - 3] = register_values[0];
		}

		if (signature == "i" || signature == "ri") {
			std::uint16_t value = integer_literal_values[0];

			program[program.size() - 1] = value & 0xff;
			program[program.size() - 2] = (value << 8) & 0xff;
		}
		
		if (signature == "rr") {
			program[program.size() - 1] = register_values[1];
		}
	}

	if (Good()) logger.Log("Instructions parsed!");
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
