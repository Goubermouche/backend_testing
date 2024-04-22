#include "address_space.h"
#include "baremetal/dialects/core_dialect.h"

namespace baremetal {
	address_space::address_space() : m_usage(0) {}

	auto address_space::allocate(u8 bytes, u8 alignment) -> i32 {
		m_usage = utility::align(m_usage + bytes, alignment);
		return -static_cast<i32>(m_usage);
	}

	auto address_space::get_usage() const -> u64 {
		return m_usage;
	}

	auto address_space::get_slot(ptr<ir::node> node) -> i32 {
		const auto it = m_slots.find(node);

		if(it != m_slots.end()) {
			return it->second;
		}

		const memory_block& local_data = node->get_data<memory_block>();
		const i32 position = allocate(local_data.size, local_data.alignment);

		m_slots[node] = position;
		return position;
	}
} // namespace baremetal

