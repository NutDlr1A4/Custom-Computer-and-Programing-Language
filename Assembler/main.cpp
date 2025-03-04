import std;
import assembler;

int main() {
	// This main function is just for testing

	std::vector<std::string> test_file_names = {
		"test"
	};

	for (std::string name : test_file_names) {
		std::string input = "Tests\\" + name + ".asm";
		std::string output = "Assembled\\" + name + "_assembled";

		std::println("Assembling {}", input);
		std::ifstream file(input, std::ios::in);
		if (!file) {
			std::println("Could not open file at {}", input);
			continue;
		}

		Assembler assembler;
		assembler.Assemble(file, true);
	}

	return 0;
}