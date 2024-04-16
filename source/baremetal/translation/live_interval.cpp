#include "live_interval.h"

namespace baremetal {
	live_interval::live_interval(const std::vector<utility::range<i32>>& ranges, reg reg)
		: ranges(ranges), m_reg(reg) {}

	live_interval::live_interval(const std::vector<utility::range<i32>>& ranges, ptr<ir::node> node)
		: ranges(ranges), node(node) {}

	void live_interval::add_range(i32 start, i32 end) {
		ASSERT(start <= end, "invalid range");
		ASSERT(!ranges.empty(), "empty ranges");

		if(ranges.back().start <= end) {
			utility::range<i32>& top = ranges.back();

			top.start = std::min(top.start, start);
			top.end = std::max(top.end, end);
		}
		else {
			ranges.push_back({ start, end });
		}
	}
} // namespace baremetal
