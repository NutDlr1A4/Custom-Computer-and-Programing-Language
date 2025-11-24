/*
*	The Generator takes in the tokens created by the lexer, does some analysis of syntax and translates the instructions
*	and identifiers. When it is done, it spits out the long awaited bytes that is the program, ready to be loaded into
*	memory and run!
*/

#pragma once

#include <map>

#include "token.h"
#include "lookahead.hpp"
#include "errorlog.h"

class Generator {
public:
	Generator(const std::vector<Token>& tokens, const ErrorLog& logger);

	std::vector<std::uint8_t> Generate();

	bool Good() const;

private:
	void DefineProgramLabels();
	void DefineDataLabels();

	void ParseInstruction();

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