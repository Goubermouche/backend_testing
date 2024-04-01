#include "x64_target.h"


namespace baremetal {
	void x64_target::select_instructions(const module_data& data) {
		SUPPRESS_C4100(data);
		 /*for(const node node : data.nodes) {
		 	switch(node.id.m_dialect) {
		 		case get_dialect_id<core_dialect>(): {
		 			std::cout << "isel core\n";
		 			break;
		 		}
		 		case get_dialect_id<affine_dialect>(): {
		 			std::cout << "isel affine\n";
		 			break;
		 		}
		 		default: {
		 			std::cerr << "unsupported_dialect\n";
		 			break;
		 		}
		 	}
		 }*/
	}
} // namespace baremetal
