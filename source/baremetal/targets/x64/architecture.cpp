#include "architecture.h"

namespace baremetal::x64 {
	auto create_architecture() -> architecture {
		architecture arch;

		arch.address_spaces.resize(1);

		arch.registers.resize(2);
		arch.registers[GPR].resize(16);
		arch.registers[XMM].resize(16);

		for(u16 i = 0; i < 16; ++i) {
			arch.registers[GPR][i] = { GPR, i };
		}

		for(u16 i = 0; i < 16; ++i) {
			arch.registers[XMM][i] = { XMM, i };
		}

		return arch;
	}
} // baremetal::x64
