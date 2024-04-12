#pragma once
#include "baremetal/intermediate_representation/dialect.h"

namespace baremetal {
	class context {
	public:
		template<derived_from_dialect_base dialect_type>
		auto get_dialect_index() -> u8 {
			constexpr u64 id = utility::get_type_id<dialect_type>();
			const auto it = m_index_map.find(id);

			if(it != m_index_map.end()) {
				return it->second;
			}

			const u8 size = static_cast<u8>(m_index_map.size());
			m_index_map[id] = size;
			return size;
		}
		// dialect type id -> module index
		std::unordered_map<u64, u8> m_index_map;
	};
} // namespace baremetal
