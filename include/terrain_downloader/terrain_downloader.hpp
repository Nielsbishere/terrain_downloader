#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <stdint.h>

using u8 = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f64 = double;

namespace terrainDownloader {
	int save(f64 beginX, f64 endX, f64 beginY, f64 endY, u32 w, u32 h, u8 startMip, u8 endMip);
}