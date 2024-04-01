#pragma once
#include <format>
#include "types.h"

namespace utility {
	class console {
	public:
		static void set_stream(std::ostream& stream) {
			s_stream = &stream;
			s_is_file_stream = false;
		}

		static void set_stream(std::ofstream& stream) {
			s_stream = &stream;
			s_is_file_stream = true;
		}

		template<typename... arguments>
		static void out(std::format_string<arguments...> fmt, arguments&&... args) {
			*s_stream << format_str(std::move(fmt), std::forward<arguments>(args)...);
		}

		template<typename... arguments>
		static void err(std::format_string<arguments...> fmt, arguments&&... args) {
			if(s_is_file_stream) {
				*s_stream << format_str(std::move(fmt), std::forward<arguments>(args)...);
			}
			else {
				std::cerr << format_str(std::move(fmt), std::forward<arguments>(args)...);
			}
		}

		static void flush() {
			*s_stream << std::flush;
		}

		static void err_flush() {
			if(s_is_file_stream) {
				*s_stream << std::flush;
			}
			else {
				std::cerr << std::flush;
			}
		}
	private:
		template<typename... arguments>
		static auto format_str(std::format_string<arguments...> fmt, arguments&&... args) -> std::string {
			return std::format(fmt, std::forward<arguments>(args)...);
		}
	private:
		inline static std::ostream* s_stream = &std::cout;
		inline static bool s_is_file_stream = false;
	};
} // namespace utility
