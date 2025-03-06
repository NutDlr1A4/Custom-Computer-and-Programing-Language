export module assembler;

import lexer;
import std;

export class Assembler {
public:
	std::vector<std::uint8_t> Assemble(std::istream& file, bool doLogging = false);

	void Log(std::string_view msg) const;
	void Error(std::string_view msg, std::string_view desc = "", bool fatal = true);

	bool Good() const;
private:
	bool logging = false;
	bool good = true;
};



std::vector<std::uint8_t> Assembler::Assemble(std::istream& content, bool doLogging) {
	logging = doLogging;

	Log("Assembling...");

	std::stringstream ss;
	ss << content.rdbuf();

	std::string s = ss.str() + "\n";

	Lexer lexer(s, logging);
	std::vector<Token> tokens = std::move(lexer.Tokenize());
	if (!lexer.Good()) {
		std::println("One or more errors have occured, and the program could not assemble succesfully.");
		good = false;
		return {};
	}

	if (Good())
		Log("Assembling complete!");

	return {};
}

void Assembler::Log(std::string_view msg) const {
	if (logging)
		std::println("[Assembler] {}", msg);
}

void Assembler::Error(std::string_view msg, std::string_view desc, bool fatal) {
	if (logging) {
		if (fatal) {
			std::println("[Assembler] Error  : {}", msg);
		}
		else {
			std::println("[Assembler] Warning: {}", msg);
		}
		if (!desc.empty()) std::println("\t{}", desc);
	}

	if (fatal)
		good = false;
}

bool Assembler::Good() const {
	return good;
}