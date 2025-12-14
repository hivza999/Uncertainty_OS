#include <stdio.h>
#include <stdint.h>

#define sector_size 512

typedef struct fat32_bios_parameter
{
	// boot record
	uint8_t bootjmp[3];
	char identifier[8];
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sector_count;
	uint8_t nb_fat;
	uint16_t root_entry_count;
	uint16_t unused0;
	uint8_t media_type;
	uint16_t unused1;
	uint16_t sectors_per_track;
	uint16_t head;
	uint32_t hidden_sector_count;
	uint32_t sector_count;

	// Extended boot record
	uint32_t fat_sector_count;
	uint16_t flags;
	uint32_t fat_version;
	uint32_t nb_cluster_root;
	uint16_t fsinfo_sector;
	uint16_t backup_boot_record_sector;
	uint8_t reserved0[12];
	uint8_t nb_drive;
	uint8_t reserved1;
	uint8_t signature0;
	uint32_t volume_ID;
	char volumme_name[11];
	char system_iditenfier[8];

	uint8_t unused2[0x1a2];
	uint16_t signature1;

} __attribute__((packed)) fat32_bios_parameter_t;

typedef struct fat32_FSinfo
{
	uint32_t signature0;
	uint8_t reserved0[480];
	uint32_t signature1;
	uint32_t last_free_cluster;
	uint32_t first_free_cluster;
	uint8_t reserved1[12];
	uint32_t signature3;

} __attribute__((packed)) fat32_FSinfo_t;

int main()
{

	FILE *fptr;
	fptr = fopen("bin/fat32.img", "wb");
	if (fptr == NULL)
	{
		printf("Couldn't open destination file\n");
		return (1);
	}
	printf("Building FAT32 image...\n");

	// Reserved sectors
	fat32_bios_parameter_t fat32_bios_parameter;
	fat32_FSinfo_t fat32_FSinfo;
	uint8_t empty_sector[512];

	for (uint16_t i = 0; i < 512; i++)
	{
		((uint8_t *)&fat32_bios_parameter)[i] = 0x00;
		((uint8_t *)&fat32_FSinfo)[i] = 0x00;
		empty_sector[i] = 0;
	}

	// BPB
	fat32_bios_parameter.bootjmp[0] = 0xeb;
	fat32_bios_parameter.bootjmp[1] = 0xfe;
	fat32_bios_parameter.bootjmp[2] = 0x90;
	for (uint8_t i; i < 8; i++)
	{
		fat32_bios_parameter.identifier[i] = "UncertFS"[i];
	}
	fat32_bios_parameter.bytes_per_sector = sector_size;
	fat32_bios_parameter.sectors_per_cluster = 0x08;
	fat32_bios_parameter.reserved_sector_count = 0x0020;
	fat32_bios_parameter.nb_fat = 2;
	fat32_bios_parameter.root_entry_count = 0;
	fat32_bios_parameter.media_type = 0xf8;
	fat32_bios_parameter.sectors_per_track = 0x003e;
	fat32_bios_parameter.head = 0x00f7;
	fat32_bios_parameter.hidden_sector_count = 0x00000800;
	fat32_bios_parameter.sector_count = 0x001ffffe;

	fat32_bios_parameter.fat_sector_count = 0x0800;
	fat32_bios_parameter.flags = 0x0000;
	fat32_bios_parameter.fat_version = 0x0000;
	fat32_bios_parameter.nb_cluster_root = 2;
	fat32_bios_parameter.fsinfo_sector = 0x0001;
	fat32_bios_parameter.backup_boot_record_sector = 0x0006;
	fat32_bios_parameter.nb_drive = 0x80;
	fat32_bios_parameter.signature0 = 0x29;
	fat32_bios_parameter.volume_ID = 0x0000abcd;
	for (uint8_t i; i < 11; i++)
	{
		fat32_bios_parameter.volumme_name[i] = "UNCERTAINTY"[i];
	}
	for (uint8_t i; i < 8; i++)
	{
		fat32_bios_parameter.system_iditenfier[i] = "FAT32   "[i];
	}

	// FSinfo
	fat32_FSinfo.signature0 = 0x41615252;
	fat32_FSinfo.signature1 = 0x61417272;
	fat32_FSinfo.last_free_cluster = -1;
	fat32_FSinfo.first_free_cluster = -1;
	fat32_FSinfo.signature3 = 0xaa550000;

	// write it
	/*
	fwrite(&fat32_bios_parameter, 512, 1, fptr); // 0
	fwrite(&fat32_FSinfo, 512, 1, fptr);				 // 1
	for (uint8_t i = 2; i < 6; i++)
	{
		fwrite(empty_sector, 512, 1, fptr); // 2-5
	}
	fwrite(&fat32_bios_parameter, 512, 1, fptr); // 6
	fwrite(&fat32_FSinfo, 512, 1, fptr);				 // 7
	for (uint8_t i = 8; i < fat32_bios_parameter.reserved_sector_count; i++)
	{
		fwrite(empty_sector, 512, 1, fptr);
	}
		*/

	// fat
	uint32_t FAT[fat32_bios_parameter.fat_sector_count * sector_size / sizeof(uint32_t)];
	for (uint32_t i = 0; i < fat32_bios_parameter.fat_sector_count * sector_size / sizeof(uint32_t); i++)
	{
		FAT[i] = 0;
	}

	FAT[0] = 0x0ffffff8;
	FAT[1] = 0x0fffffff;

	// 2 copies are written
	fwrite(FAT, sizeof(FAT), 1, fptr);
	fwrite(FAT, sizeof(FAT), 1, fptr);

	return (0);
}