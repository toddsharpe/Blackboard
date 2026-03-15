#pragma once

#include "Assert.h"
#include "Types.h"
#include <type_traits>
#include <cstdint>
#include <cstring>

enum class TypeCode : uint8_t
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

const char TypeCodeNames[][4] = 
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

template <typename T>
constexpr TypeCode GetTypeCode()
{
	if constexpr (std::is_same<uint8_t, T>::value)
		return TypeCode::u8;
	else if constexpr (std::is_same<uint16_t, T>::value)
		return TypeCode::u16;
	else if constexpr (std::is_same<uint32_t, T>::value)
		return TypeCode::u32;
	else if constexpr (std::is_same<uint64_t, T>::value)
		return TypeCode::u64;

	else if constexpr (std::is_same<int8_t, T>::value)
		return TypeCode::i8;
	else if constexpr (std::is_same<int16_t, T>::value)
		return TypeCode::i16;
	else if constexpr (std::is_same<int32_t, T>::value)
		return TypeCode::i32;
	else if constexpr (std::is_same<int64_t, T>::value)
		return TypeCode::i64;

	else if constexpr (std::is_same<float, T>::value)
		return TypeCode::f32;
	else if constexpr (std::is_same<double, T>::value)
		return TypeCode::f64;

	else if constexpr (std::is_same<str_t, T>::value)
		return TypeCode::str;

	else
	{
		static_assert(false, "Not implemented ");
		return static_cast<TypeCode>(0);
	}
}

constexpr size_t GetTypeSize(const TypeCode code)
{
	switch (code)
	{
		case TypeCode::u8:
		case TypeCode::i8:
			return sizeof(uint8_t);

		case TypeCode::u16:
		case TypeCode::i16:
			return sizeof(uint16_t);

		case TypeCode::u32:
		case TypeCode::i32:
			return sizeof(uint32_t);

		case TypeCode::u64:
		case TypeCode::i64:
			return sizeof(uint64_t);

		case TypeCode::f32:
			return sizeof(float);

		case TypeCode::f64:
			return sizeof(double);

		case TypeCode::str:
			return sizeof(str_t);

		default:
			ASSERT(false);
			return 0;
	}
}

inline const char* GetTypeName(const TypeCode code)
{
	return TypeCodeNames[static_cast<int>(code)];
}
