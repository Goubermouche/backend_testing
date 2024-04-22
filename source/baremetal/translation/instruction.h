#pragma once
#include "baremetal/intermediate_representation/node_hierarchy/node.h"
#include "baremetal/translation/reg.h"

namespace baremetal {
	struct virtual_value {
		reg virtual_register;
		i32 use_count;
	};

	struct phi_value {
		ptr<ir::node> phi;
		ptr<ir::node> node;

		reg destination;
		reg source;
	};

	struct scheduled_phi {
		ptr<ir::node> phi;
		ptr<ir::node> node;
	};

	struct scheduled_node {
		ptr<scheduled_node> parent;
		ptr<ir::node> node;
		ptr<ir::user> antis;
		i32 index;
	};

	enum instruction_flags {
		NONE      = 0,
		RETURN    = 1 << 0,
		IMMEDIATE = 1 << 1,
		MEMORY    = 1 << 2,
		GLOBAL    = 1 << 3,
		INDEXED   = 1 << 4,
		ABSOLUTE  = 1 << 5,
	};

	inline instruction_flags& operator|=(instruction_flags& a, instruction_flags b) {
		a = static_cast<instruction_flags>(static_cast<int>(a) | static_cast<int>(b));
		return a;
	}

	class instruction {
	public:
		instruction();
	public:
		instruction_flags flags;
		ptr<instruction> next;
	};
} // namespace baremetal
