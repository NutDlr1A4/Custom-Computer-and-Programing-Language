#include "assembler.h"

int main() {
	std::vector<std::string> test_file_names = {
		"test",
		//"helloworld"
	};

	for (std::string name : test_file_names) {
		std::string input = "Tests\\" + name + ".asm";
		std::string output = "Assembled\\" + name + "_assembled";

		Assembler assembler(std::cout, LoggingLevel::ALL);
		assembler.AssembleToFile(input, output, Assembler::Format::LOGISIM_2_RAW);
	}

	return 0;
}