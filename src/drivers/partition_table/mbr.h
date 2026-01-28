#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	uint32_t LBA_start;
	uint32_t sector_count;
	uint8_t partition_id;
	bool present;
} partition_t;

int mbr_get_partition(partition_t paritions[4]);