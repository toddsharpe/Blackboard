#pragma once

#include <cstdint>
#include <cstddef>

//Constants from https://crccalc.com/
namespace Crc
{
	struct Config16
	{
		uint16_t Check;
		uint16_t Poly;
		uint16_t Init;
		bool RefIn;
		bool RefOut;
		uint16_t XorOut;
	};

	static constexpr Config16 SDLC
	{
		.Check = 0x906E,
		.Poly = 0x1021,
		.Init = 0xFFFF,
		.RefIn = true,
		.RefOut = true,
		.XorOut = 0xFFFF
	};

	struct Config8
	{
		uint8_t Check;
		uint8_t Poly;
		uint8_t Init;
		//bool RefIn;
		//bool RefOut;
		//uint8_t XorOut;
	};

	static constexpr Config8 NRSC_5
	{
		.Check = 0xF7,
		.Poly = 0x31,
		.Init = 0xFF,
		//.RefIn = false,
		//.RefOut = false,
		//.XorOut = 0
	};

	uint16_t Calculate(const void* const data, const size_t length, const Config16& config, const uint16_t partialCrc = 0);

	uint8_t Calc8(const void* const data, const size_t length, const Config8& config);

	uint32_t Calc32(const void* const data, const size_t length);
	uint32_t Calc32(const char* const c_str);
}
