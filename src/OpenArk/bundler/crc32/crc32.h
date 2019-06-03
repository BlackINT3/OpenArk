#pragma once
#include <stdint.h>

uint32_t crc32(const void *buf, size_t size, uint32_t init_crc = 0);