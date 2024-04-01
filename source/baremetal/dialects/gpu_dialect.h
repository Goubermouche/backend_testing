#pragma once
#include "baremetal/intermediate_representation/dialect.h"

namespace baremetal {
	class gpu_dialect : public dialect<gpu_dialect> {
	public:
		auto get_label(u16 id) const -> std::string_view override;
		auto get_color(u16 id) const -> std::string_view override;

		auto create_gpu_test(ptr<ir::node> node) -> ptr<ir::node>;
	};
} // namespace baremetal
