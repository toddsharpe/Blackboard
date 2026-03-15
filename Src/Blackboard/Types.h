#pragma once

#include <cstdint>
#include <cstring>

struct str_t
{
	constexpr str_t() : buffer()
	{

	}

	str_t(const char* str) : buffer()
	{
		memcpy(buffer, str, sizeof(buffer));
	}

	const char* c_str() const
	{
		return buffer;
	}

	static constexpr size_t size = 64;
	char buffer[size];
};
