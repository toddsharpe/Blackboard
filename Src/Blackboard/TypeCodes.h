#pragma once

#include <type_traits>

constexpr char TypeCodeNames[][4] =
{
	"u8",
	"u16",
	"u32",
	"u64",
	"i8",
	"i16",
	"i32",
	"i64",
	"f32",
	"f64",
	"str"
};

enum class TypeCode
{
	u8,
	u16,
	u32,
	u64,
	i8,
	i16,
	i32,
	i64,
	f32,
	f64,
	str
};

template <typename T>
constexpr TypeCode GetTypeCode()
{
	if constexpr (std::is_same<T, uint8_t>::value)
		return TypeCode::u8;
	else if constexpr (std::is_same<T, uint16_t>::value)
		return TypeCode::u16;
	else if constexpr (std::is_same<T, uint32_t>::value)
		return TypeCode::u32;
	else if constexpr (std::is_same<T, uint64_t>::value)
		return TypeCode::u64;
	else if constexpr (std::is_same<T, int8_t>::value)
		return TypeCode::iu8;
	else if constexpr (std::is_same<T, int16_t>::value)
		return TypeCode::i16;
	else if constexpr (std::is_same<T, int32_t>::value)
		return TypeCode::i32;
	else if constexpr (std::is_same<T, int64_t>::value)
		return TypeCode::i64;
	else if constexpr (std::is_same<T, float>::value)
		return TypeCode::f32;
	else if constexpr (std::is_same<T, double>::value)
		return TypeCode::f64;
	else if constexpr (std::is_same<T, const char*>::value)
		return TypeCode::str;
	else
		static_assert(false);
}

inline const char* GetTypeName(const TypeCode code)
{
	return TypeCodeNames[static_cast<int>(code)];
}

