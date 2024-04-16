#pragma once
#include "baremetal/translation/reg.h"
#include "baremetal/intermediate_representation/node_hierarchy/node.h"

#include <utility/range.h>

namespace baremetal {
	class live_interval {
	public:
		live_interval() = default;
		live_interval(const std::vector<utility::range<i32>>& ranges, reg reg);
		live_interval(const std::vector<utility::range<i32>>& ranges, ptr<ir::node> node);

		void add_range(i32 start, i32 end);

		std::vector<utility::range<i32>> ranges;
		ptr<ir::node> node;

		ptr<live_interval> base;
		reg m_reg;
	};
} // namespace baremetal
