#pragma once
#include "baremetal/translation/reg.h"
#include "baremetal/intermediate_representation/node_hierarchy/node.h"

#include <utility/range.h>

namespace baremetal {
	class live_interval {
	public:
		live_interval() = default;
		live_interval(const std::vector<utility::range<i32>>& ranges, reg reg);

		void add_range(i32 start, i32 end);
	private:
		std::vector<utility::range<i32>> m_ranges;

		ptr<live_interval> m_base;
		ptr<ir::node> m_node;

		reg m_reg;
	};
} // namespace baremetal
