export module assembler;

import std;

import lexer;
import generation;

import errorlog;

export class Assembler {
public:
	Assembler(std::ostream& ostream_log, LoggingLevel logging_level);

	std::vector<std::uint8_t> Assemble(std::istream& file);
	std::vector<std::uint8_t> Assemble(const std::string& input_filepath);
	std::vector<std::uint8_t> AssembleToFile(const std::string& input_filepath, const std::string& output_filepath);

	bool Good() const;
private:
	ErrorLog logger;
};

Assembler::Assembler(std::ostream& ostream_log, LoggingLevel logging_level) 
	:
	logger("Assembler", ostream_log, logging_level)
{}

std::vector<std::uint8_t> Assembler::Assemble(std::istream& file) {
	logger.Log("Assembling...");

	std::stringstream ss;
	ss << file.rdbuf();

	std::string s = ss.str() + "\n";

	Lexer lexer(s, logger);
	std::vector<Token> tokens = std::move(lexer.Tokenize());
	if (!lexer.Good()) {
		logger.FinalError();
		return {};
	}

	Generator gen(tokens, logger);
	std::vector<std::uint8_t> data = std::move(gen.Generate());
	if (!gen.Good()) {
		logger.FinalError();
		return {};
	}

	logger.Log("Assembling complete!");

	return data;
}

std::vector<std::uint8_t> Assembler::Assemble(const std::string& input_filepath) {
	logger.Log("Assembling file at \"" + input_filepath + "\"");

	std::ifstream file(input_filepath, std::ios::in);
	if (!file) {
		logger.Error("Could not open file at path \"" + input_filepath + "\"");

		if (!Good()) {
			logger.FinalError();
			return {};
		}
	}
	
	return std::move(Assemble(file));
}

std::vector<std::uint8_t> Assembler::AssembleToFile(const std::string& input_filepath, const std::string& output_filepath) {
	logger.Error("AssembleToFile() has not been implemented yet!");
	return {};
}

bool Assembler::Good() const {
	return logger.Good();
}

