#include "Crc.h"
#include <cstdint>
#include <limits.h>
#include <cstdio>
#include <cstring>

namespace
{
	uint8_t reflect(const uint8_t data)
	{
		uint8_t _data = data;
		_data = (_data & 0xF0) >> 4 | (_data & 0x0F) << 4;
		_data = (_data & 0xCC) >> 2 | (_data & 0x33) << 2;
		_data = (_data & 0xAA) >> 1 | (_data & 0x55) << 1;
		return _data;
	}

	uint16_t reflect(uint16_t data)
	{
		const uint8_t high = reflect((uint8_t)(data >> 8));
		const uint8_t low = reflect((uint8_t)data);
		return (uint16_t)(low << 8) | high;
	}
}

// https://barrgroup.com/downloads/code-crc-c
uint16_t Crc::Calculate(const void* const data, const size_t length, const Crc::Config16& config, const uint16_t partialCrc)
{
	uint16_t remainder = config.Init;
	if (partialCrc != 0)
	{
		const uint16_t undone = partialCrc ^ config.XorOut;
		remainder = config.RefOut ? reflect(undone) : undone;
	}

	const uint8_t* const bytes = (uint8_t*)data;
	for (size_t i = 0; i < length; i++)
	{
		remainder ^= (config.RefIn ? reflect(bytes[i]) : bytes[i]) << 8;
		for (size_t j = 0; j < CHAR_BIT; j++)
		{
			if (remainder & 0x8000)
				remainder = (remainder << 1) ^ config.Poly;
			else
				remainder = (remainder << 1);
		}
	}

	/*
	 * The final remainder is the CRC result.
	 */
	const uint16_t out = config.RefOut ? reflect(remainder) : remainder;
	return (out ^ config.XorOut);
}

uint8_t Crc::Calc8(const void* const data, const size_t length, const Config8& config)
{
	uint8_t crc = config.Init;
	const uint8_t* const bytes = reinterpret_cast<const uint8_t*>(data);

	/* Calculate 8-bit checksum for given polynomial */
	for (uint16_t index = 0; index < length; index++)
	{
		crc ^= bytes[index];
		for (uint8_t crc_bit = 8U; crc_bit > 0U; crc_bit--)
		{
			crc = ((crc & 0x80U) != 0U) ? ((crc << 1) ^ config.Poly) : (crc << 1);
		}
	}

	return crc;
}

uint32_t Crc::Calc32(const char* const c_str)
{
	const size_t size = strlen(c_str);
	return Calc32(c_str, size);
}

//CRC-32/ISO-HDLC
//https://web.archive.org/web/20190108202303/http://www.hackersdelight.org/hdcodetxt/crc.c.txt
uint32_t Crc::Calc32(const void* const data, const size_t length)
{
	unsigned int byte, crc, mask;

	crc = 0xFFFFFFFF;

	const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
	for (size_t i = 0; i < length; i++)
	{
		byte = bytes[i];
		crc = crc ^ byte;
		for (int j = 7; j >= 0; j--)
		{
			const int8_t t1 = crc & 1;
			const int8_t temp = -(t1);
			mask = temp;
			crc = (crc >> 1) ^ (0xEDB88320 & mask);
		}
	}
	return ~crc;
}
