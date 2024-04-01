#pragma once
#include "types.h"

namespace utility {
	template<typename type>
	struct range {
		static auto max() -> range {
			return {
				std::numeric_limits<type>::max(),
				std::numeric_limits<type>::max()
			};
		}

		static auto min() -> range {
			return {
				std::numeric_limits<type>::min(),
				std::numeric_limits<type>::min()
			};
		}

		type start;
		type end;
	};
} // namespace utility
