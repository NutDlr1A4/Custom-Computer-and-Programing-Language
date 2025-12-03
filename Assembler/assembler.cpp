#include "assembler.h"

#include <bitset>
#include <iomanip>

Assembler::Assembler() = default;

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

std::vector<std::uint8_t> Assembler::AssembleToFile(const std::string& input_filepath, const std::string& output_filepath, Format format) {
	std::ofstream file;
	if (format == Format::RAW) {
		file.open(output_filepath, std::ios::out | std::ios::binary);
	}
	else {
		file.open(output_filepath, std::ios::out);
	}

	if (!file) {
		logger.Error("Could not create file at path \"" + output_filepath + "\"");
		return {};
	}

	std::vector<std::uint8_t> data = std::move(Assemble(input_filepath));
	if (!Good()) return {};

	if (format == Format::RAW) {
		for (std::uint8_t i : data) {
			file << i;
		}
	}
	else if (format == Format::BIN) {
		int ctr = 0;
		for (std::uint8_t i : data) {
			file << std::bitset<8>(i).to_string();

			if (ctr < 7) {
				file << ' ';
				ctr++;
			}
			else {
				file << '\n';
				ctr = 0;
			}
		}
	}
	else if (format == Format::HEX) {
		int ctr = 0;
		file << std::hex;
		for (std::uint8_t i : data) {
			file << std::setfill('0') << std::setw(2) << (int)i;;

			if (ctr < 7) {
				file << ' ';
				ctr++;
			}
			else {
				file << '\n';
				ctr = 0;
			}
		}
	}
	else if (format == Format::LOGISIM_2_RAW) {
		file << "v2.0 raw\n";

		int ctr = 0;
		file << std::hex;
		for (std::uint8_t i : data) {
			file << std::setfill('0') << std::setw(2) << (int)i;;

			if (ctr < 3) {
				file << ' ';
				ctr++;
			}
			else {
				file << '\n';
				ctr = 0;
			}
		}
	}

	return data;
}

bool Assembler::Good() const {
	return logger.Good();
}

