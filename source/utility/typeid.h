// Implementation based on a ctti (https://github.com/Manu343726/ctti)(MIT)

#pragma once
#include "utility/types.h"

#if defined(__clang__)
#define TYPE_PRETTY_FUNCTION_PREFIX "utility::detail::constexpr_string utility::pretty_function::type() [T = "
#define TYPE_PRETTY_FUNCTION_SUFFIX "]"
#elif defined(__GNUC__) && !defined(__clang__)
#define TYPE_PRETTY_FUNCTION_PREFIX "constexpr utility::detail::constexpr_string utility::pretty_function::type() [with T = "
#define TYPE_PRETTY_FUNCTION_SUFFIX "]"
#elif defined(_MSC_VER)
#define TYPE_PRETTY_FUNCTION_PREFIX "struct utility::detail::constexpr_string __cdecl utility::pretty_function::type<"
#define TYPE_PRETTY_FUNCTION_SUFFIX ">(void)"
#else
#error "compiler not supported"
#endif

#define TYPE_PRETTY_FUNCTION_LEFT (sizeof(TYPE_PRETTY_FUNCTION_PREFIX) - 1)
#define TYPE_PRETTY_FUNCTION_RIGHT (sizeof(TYPE_PRETTY_FUNCTION_SUFFIX) - 1)

#if defined(__clang__)
#define PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(__GNUC__) && !defined(__clang__)
#define PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define PRETTY_FUNCTION __FUNCSIG__
#else
#error "compiler not supported"
#endif

namespace utility {
	namespace detail {
		constexpr u64 fnv_basis = 14695981039346656037ull;
		constexpr u64 fnv_prime = 1099511628211ull;

		constexpr auto fnv1a_hash(u64 n, const char* str, u64 hash = fnv_basis) -> u64 {
			return n > 0 ? fnv1a_hash(n - 1, str + 1, (hash ^ *str) * fnv_prime) : hash;
		}

		template<u64 size>
		constexpr auto fnv1a_hash(const char(&array)[size]) -> u64 {
			return fnv1a_hash(size - 1, &array[0]);
		}

		template<typename left_type, typename right_type>
		constexpr auto equal_range(left_type left_begin, left_type left_end, right_type right_begin, right_type right_end) -> bool {
			return (left_begin != left_end && right_begin != right_end) ? *left_begin == *right_begin && 
				equal_range(left_begin + 1, left_end, right_begin + 1, right_end) : (left_begin == left_end && right_begin == right_end);
		}

		template<typename T>
		struct type_tag {
			constexpr type_tag() = default;
			using type = T;
		};
	} // namespace detail
	
	class constexpr_string {
	public:
		template<u64 size>
		constexpr constexpr_string(const char(&str)[size]) : constexpr_string{ &str[0], size - 1 } {}
		constexpr constexpr_string(const char* begin, u64 length) : m_value{ begin }, m_size{ length } {}
		constexpr constexpr_string(const char* begin, const char* end) : constexpr_string{ begin, static_cast<u64>(end - begin) } {}
		constexpr constexpr_string(const char* begin) : constexpr_string{ begin, calculate_size(begin) } {}

		static constexpr auto calculate_size(const char* str) -> u64 {
			return *str ? 1 + calculate_size(str + 1) : 0;
		}

		constexpr auto get_size() const -> u64 {
			return m_size;
		}

		constexpr auto hash() const -> u64 {
			return detail::fnv1a_hash(get_size(), begin());
		}

		constexpr auto begin() const -> const char* {
			return m_value;
		}

		constexpr auto end() const -> const char* {
			return m_value + m_size;
		}

		constexpr auto operator[](u64 i) const -> char {
			return m_value[i];
		}

		constexpr auto operator()(u64 i) const -> const char* {
			return m_value + i;
		}

		constexpr auto operator()(u64 begin, u64 end) const -> constexpr_string {
			return { m_value + begin, m_value + end };
		}

		constexpr auto pad(u64 begin_offset, u64 end_offset) const -> constexpr_string {
			return operator()(begin_offset, get_size() - end_offset);
		}

		friend auto operator<<(std::ostream& os, const constexpr_string& str) -> std::ostream& {
			for(const char c : str) {
				os << c;
			}

			return os;
		}

	private:
		const char* m_value;
		u64 m_size;
	};

	constexpr auto operator==(const constexpr_string& lhs, const constexpr_string& rhs) -> bool {
		return detail::equal_range(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

	struct type_id_t {
		constexpr type_id_t(const constexpr_string& name) : m_name{ name } {}
		constexpr type_id_t() : type_id_t{ "void" } {}

		auto operator=(const type_id_t&) ->type_id_t& = default;

		constexpr auto hash() const -> u64 {
			return m_name.hash();
		}

		friend constexpr auto operator==(const type_id_t& lhs, const type_id_t& rhs) -> bool {
			return lhs.hash() == rhs.hash();
		}

		friend constexpr auto operator!=(const type_id_t& lhs, const type_id_t& rhs) -> bool {
			return !(lhs == rhs);
		}
	private:
		constexpr_string m_name;
	};

	namespace detail {
		constexpr auto pad_left(const constexpr_string& str) -> constexpr_string {
			return (str.get_size() > 0 && str[0] == ' ') ? pad_left(str(1, str.get_size())) : str;
		}

		constexpr auto filter_prefix(const constexpr_string& str, const constexpr_string& prefix) -> constexpr_string {
			return str.get_size() >= prefix.get_size() ? (str(0, prefix.get_size()) == prefix ? str(prefix.get_size(), str.get_size()) : str) : str;
		}

		constexpr auto filter_struct(const constexpr_string& type_name) -> constexpr_string {
			return pad_left(filter_prefix(pad_left(type_name), "struct"));
		}

		constexpr auto filter_class(const constexpr_string& type_name) -> constexpr_string {
			return pad_left(filter_prefix(pad_left(type_name), "class"));
		}

		constexpr auto filter_typename_prefix(const constexpr_string& type_name) -> constexpr_string {
			return filter_struct(filter_class(type_name));
		}

		namespace pretty_function {
			template<typename T>
			constexpr constexpr_string type() {
				return { PRETTY_FUNCTION };
			}
		}

		template<typename type, typename = void>
		struct nameof_impl {
			static constexpr auto apply() -> constexpr_string {
				return detail::filter_typename_prefix(pretty_function::type<type>().pad(
					TYPE_PRETTY_FUNCTION_LEFT, 
					TYPE_PRETTY_FUNCTION_RIGHT
				));
			}
		};

		template<typename type>
		constexpr auto nameof(type_tag<type>) -> constexpr_string {
			return nameof_impl<type>::apply();
		}
	} // namespace detail

	template<typename type>
	constexpr auto nameof() -> constexpr_string {
		return detail::nameof(detail::type_tag<type>());
	}

	template<typename type>
	constexpr auto type_id() -> type_id_t {
		return { utility::nameof<type>() };
	}

	template<typename type>
	constexpr auto get_type_id() -> u64 {
		return type_id<type>().hash();
	}
} // namespace utility
