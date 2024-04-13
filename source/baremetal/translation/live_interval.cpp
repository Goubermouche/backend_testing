#include "live_interval.h"

namespace baremetal {
	live_interval::live_interval(const std::vector<utility::range<i32>>& ranges, reg reg)
		: m_ranges(ranges), m_reg(reg) {}

	void live_interval::add_range(i32 start, i32 end) {
		ASSERT(start <= end, "invalid range");
		ASSERT(!m_ranges.empty(), "empty ranges");

		if(m_ranges.back().start <= end) {
			utility::range<i32>& top = m_ranges.back();

			top.start = std::min(top.start, start);
			top.end = std::max(top.end, end);
		}
		else {
			m_ranges.push_back({ start, end });
		}
	}
} // namespace baremetal
