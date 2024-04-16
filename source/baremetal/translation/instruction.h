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

	class instruction {
	public:
		ptr<instruction> next;
	};
} // namespace baremetal
