#pragma once
#include "baremetal/intermediate_representation/node_hierarchy/node.h"

namespace baremetal {
	using namespace utility::types;

	class address_space {
	public:
		address_space();

		auto allocate(u8 bytes, u8 alignment) -> i32;
		auto get_usage() const -> u64;

		auto get_slot(ptr<ir::node> node) -> i32;
	protected:
		std::unordered_map<ptr<ir::node>, i32> m_slots;
		u64 m_usage;
	};
} // namespace baremetal
