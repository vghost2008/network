#pragma once
#include <stdint.h>

struct __attribute__((packed)) TraceData
{
	static constexpr uint32_t kStartCode = 0x72547254;
	uint32_t start_code=0x72547254;
	int16_t power=100;
	uint16_t counter=0;
	//int16_t data; /*PTS个*/
};
