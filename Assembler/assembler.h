#pragma once

#include <fstream>
#include <sstream>

#include "lexer.h"
#include "generation.h"

class Assembler {
public:
	enum class Format {
		RAW, // Raw data
		BIN, // Binary
		HEX, // Hexadecimal
		LOGISIM_2_RAW // Logisims ROM import format (v2.0 raw)
	};

public:
	Assembler(); // No logging
	Assembler(std::ostream& ostream_log, LoggingLevel logging_level); // Logging

	std::vector<std::uint8_t> Assemble(std::istream& file);
	std::vector<std::uint8_t> Assemble(const std::string& input_filepath);
	std::vector<std::uint8_t> AssembleToFile(const std::string& input_filepath, const std::string& output_filepath, Format format);

	bool Good() const;
private:
	ErrorLog logger;
};