#pragma once
#include "baremetal/intermediate_representation/dialect.h"

namespace baremetal {
	class gpu_dialect : public dialect {
	public:
		[[nodiscard]] auto get_label(ptr<ir::node> node) const -> std::string_view override;

		auto create_gpu_test(ptr<ir::node> node) -> ptr<ir::node>;
	};
} // namespace baremetal
