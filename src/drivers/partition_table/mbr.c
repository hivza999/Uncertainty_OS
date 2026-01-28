#include "../storage/ata.h"
#include "../display/text.h"
#include "mbr.h"

typedef struct
{
	uint8_t boot_indicator;
	uint8_t CHS_start[3];
	uint8_t system_id;
	uint8_t CHS_end[3];
	uint32_t LBA_start;
	uint32_t sector_count;
} __attribute__((packed)) mbr_partition_t;

int mbr_get_partition(partition_t paritions[4])
{
	uint8_t buffer[512];
	pio_read_packet_t pio_read_packet;
	pio_read_packet.LBA = 0 | 0xe0000000;
	pio_read_packet.sector_count = 1;
	pio_read_packet.buffer = buffer;

	if (ATA_PIO_read(&pio_read_packet))
	{
		print("Unable to read disk", 0x0f);
		return (1);
	}

	mbr_partition_t *mbr_paritions = (mbr_partition_t *)(buffer + 0x1be);

	for (uint8_t i = 0; i < 4; i++)
	{
		if (mbr_paritions[i].LBA_start == 0) // A partition can't start on sector 0 (boot sector)
		{
			paritions[i].present = false;
		}
		else
		{
			paritions[i].present = true;
			paritions[i].LBA_start = mbr_paritions[i].LBA_start;
			paritions[i].sector_count = mbr_paritions[i].sector_count;
			paritions[i].partition_id = i;
		}
	}

	return (0);
}