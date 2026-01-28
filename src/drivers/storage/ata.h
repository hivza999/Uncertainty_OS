#pragma once
#include <stdint.h>

#define ATA_Drive_select_master 0xe0000000
#define ATA_Drive_select_salve 0xf0000000

typedef struct
{
	uint32_t LBA; // high 4 bits (28~31) are bits are used for drive select
	void *buffer;
	uint8_t sector_count;
} __attribute__((packed)) pio_read_packet_t;

int ATA_PIO_read(pio_read_packet_t *pio_read_packet);
uint32_t ATA_init();
