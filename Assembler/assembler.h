#pragma once

#include <fstream>
#include <sstream>

#include "lexer.h"
#include "generation.h"

class Assembler {
public:
	Assembler(std::ostream& ostream_log, LoggingLevel logging_level);

	std::vector<std::uint8_t> Assemble(std::istream& file);
	std::vector<std::uint8_t> Assemble(const std::string& input_filepath);
	std::vector<std::uint8_t> AssembleToFile(const std::string& input_filepath, const std::string& output_filepath);

	bool Good() const;
private:
	ErrorLog logger;
};