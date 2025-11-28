#pragma once

#include <vector>
#include <map>

#include "token.h"

/*
struct Instruction {
	std::uint8_t opcode;
	std::string lit;

	std::string signature;
};

inline const std::map<std::pair<std::string, std::string>, std::uint8_t> valid_instruction_map = {
	{ { "ldw", "ri" }, 0x00 },
	{ { "ldw", "rr" }, 0x01 },
	{ { "ldb", "ri" }, 0x02 },
	{ { "ldb", "rr" }, 0x03 },
	{ { "ld",  "ri" }, 0x04 },
	{ { "ld",  "rr" }, 0x05 }
};
*/

struct InstructionVariant {
	std::string signature;
	std::uint8_t opcode;
};

inline const std::map<std::string, std::vector<InstructionVariant>> valid_instruction_map = {
	{ "ldw", { { "ri", 0x00 }, { "rr", 0x01 } } },
	{ "ldb", { { "ri", 0x02 }, { "rr", 0x03 } } },
	{ "ld",  { { "ri", 0x04 }, { "rr", 0x05 } } },

	{ "stw", { { "ri", 0x10 }, { "rr", 0x11 } } },
	{ "stb", { { "ri", 0x12 }, { "rr", 0x13 } } },

	{ "cmp", { { "ri", 0x20 }, { "rr", 0x21 } } },

	{ "beq", { { "i", 0x30 } } },
	{ "bne", { { "i", 0x31 } } },
	{ "bgt", { { "i", 0x32 } } },
	{ "ble", { { "i", 0x33 } } },
	{ "blt", { { "i", 0x34 } } },
	{ "bge", { { "i", 0x35 } } },
	{ "bcf", { { "i", 0x36 } } },
	{ "bnc", { { "i", 0x37 } } },
	{ "bpr", { { "i", 0x38 } } },
	{ "bpu", { { "i", 0x39 } } },
	{ "bra", { { "i", 0x3a } } },

	{ "add", { { "ri", 0x40 }, { "rr", 0x41 } } },
	{ "sub", { { "ri", 0x42 }, { "rr", 0x43 } } },
	{ "mul", { { "ri", 0x44 }, { "rr", 0x45 } } },
	{ "div", { { "ri", 0x46 }, { "rr", 0x47 } } },
	{ "mod", { { "ri", 0x48 }, { "rr", 0x49 } } },
	{ "and", { { "ri", 0x4a }, { "rr", 0x4b } } },
	{ "or",  { { "ri", 0x4c }, { "rr", 0x4d } } },
	{ "shr", { { "ri", 0x4e }, { "rr", 0x4f } } },
	{ "shl", { { "ri", 0x50 }, { "rr", 0x51 } } },
	{ "not", { { "r", 0x52 } } },
	{ "neg", { { "r", 0x53 } } },

	{ "wport", { { "i", 0x60 }, { "r", 0x61 } } },
	{ "port",  { { "i", 0x62 } } },
	{ "rport", { { "r", 0x63 } } },

	{ "hlt", { { "", 0xff } } }
};

inline const std::map<std::string, std::uint8_t> valid_register_map = {
	{ "r0",  0x00 },
	{ "r1",  0x01 },
	{ "r2",  0x02 },
	{ "r3",  0x03 },
	{ "r4",  0x04 },
	{ "r5",  0x05 },
	{ "r6",  0x06 },
	{ "r7",  0x07 },

	{ "ra",  0x00 },
	{ "rb",  0x01 },
	{ "rc",  0x02 },
	{ "rd",  0x03 },
	{ "re",  0x04 },
	{ "rsp", 0x05 },
	{ "rsb", 0x06 },
	{ "rip", 0x07 }
};

// Checks if a string is a reserved keyword (i.e. registers or instruction names)
inline bool IsReservedKeyword(const std::string& str) {
	if (valid_register_map.find(str) != valid_register_map.end()) {
		return true;
	}

	if (valid_instruction_map.find(str) != valid_instruction_map.end()) {
		return true;
	}

	return false;
}