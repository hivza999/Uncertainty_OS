#include <stdio.h>
#include <stdint.h>

#define sector_size 512
#define sector_per_cluster 0x08

#define Cluster_size sector_per_cluster *sector_size
#define fat_nb_sector 0x0800

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_LABEL 0x08
#define ATTR_DIRECTORY 0x10

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
	uint16_t fat_version;
	uint32_t nb_root_cluster;
	uint16_t fsinfo_sector;
	uint16_t backup_boot_record_sector;
	uint8_t reserved0[12];
	uint8_t nb_drive;
	uint8_t reserved1;
	uint8_t signature0;
	uint32_t volume_ID;
	char volumme_name[11];
	char system_iditenfier[8];

	uint8_t unused2[0x1a4];
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

typedef struct DirEntry
{
	char Name[8];
	char Extention[3];
	uint8_t Attribute0;
	uint8_t Attribute1;
	uint8_t create_time_ms;
	uint16_t create_time;
	uint16_t create_date;
	uint16_t access_date;
	uint16_t cluster_nb_high;
	uint16_t modified_time;
	uint16_t modified_date;
	uint16_t cluster_nb_low;
	uint32_t size;

} __attribute__((packed)) DirEntry_t;

int add_entry(DirEntry_t *dir, char *name, uint8_t atttributes, uint32_t cluster_nb, uint32_t size);
DirEntry_t *mkdir(DirEntry_t *dir, char *name);

uint32_t FAT[fat_nb_sector * sector_size / sizeof(uint32_t)];
uint8_t Clusters[16][Cluster_size];

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
	fat32_bios_parameter.sectors_per_cluster = sector_per_cluster;
	fat32_bios_parameter.reserved_sector_count = 0x0020;
	fat32_bios_parameter.nb_fat = 2;
	fat32_bios_parameter.root_entry_count = 0;
	fat32_bios_parameter.media_type = 0xf8;
	fat32_bios_parameter.sectors_per_track = 0x003e;
	fat32_bios_parameter.head = 0x00f7;
	fat32_bios_parameter.hidden_sector_count = 0x00000800;
	fat32_bios_parameter.sector_count = 0x001ffffe;

	fat32_bios_parameter.fat_sector_count = fat_nb_sector;
	fat32_bios_parameter.flags = 0x0000;
	fat32_bios_parameter.fat_version = 0x0000;
	fat32_bios_parameter.nb_root_cluster = 2;
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

	// fat
	for (uint32_t i = 0; i < fat_nb_sector * sector_size / sizeof(uint32_t); i++)
	{
		FAT[i] = 0;
	}

	FAT[0] = 0x0ffffff8;
	FAT[1] = 0x0fffffff;

	// clear cluster
	for (uint32_t i = 0; i < sizeof(Clusters); i++)
	{
		((uint8_t *)Clusters)[i] = 0;
	}

	DirEntry_t *root;
	root = (DirEntry_t *)&Clusters[0];
	FAT[2] = 0x0fffffff;

	add_entry(root, "UNCERTAINTY", ATTR_VOLUME_LABEL, 0, 0);

	DirEntry_t *folder;
	folder = mkdir(root, "FOLDER     ");

	FAT[4] = 5;
	FAT[5] = 0x0fffffff;

	add_entry(folder, "FILE    TXT", 0, 4, Cluster_size + 13);
	add_entry(folder, "LOOP       ", ATTR_DIRECTORY, 3, 0);

	for (uint32_t i = 0; i < Cluster_size; i++)
	{
		Clusters[2][i] = "Writing Data!\n"[i % 14];
	}
	for (uint32_t i = 0; i < 13; i++)
	{
		Clusters[3][i] = "\nNew Cluster!"[i];
	}

	// 2 copies are written
	fwrite(FAT, sizeof(FAT), 1, fptr);
	fwrite(FAT, sizeof(FAT), 1, fptr);

	fwrite(Clusters, Cluster_size * 16, 1, fptr);

	printf("Image builded successfully\n");
	return (0);
}

DirEntry_t *mkdir(DirEntry_t *dir, char *name)
{
	int id = 2;
	uint32_t parent_cluster_nb;
	while (FAT[id] != 0)
	{
		id++;
	}
	FAT[id] = 0x0fffffff;
	add_entry(dir, name, ATTR_DIRECTORY, id, 0);

	if (dir[0].Name == ".       ")
	{
		parent_cluster_nb = dir[0].cluster_nb_high << 16 + dir[0].cluster_nb_low;
	}
	else
	{
		if (dir[0].Attribute0 == ATTR_VOLUME_LABEL)
		{
			parent_cluster_nb = 0;
		}
		else
		{
			printf("Error,\nInvalid parent directory\n");
		}
	}

	DirEntry_t *new_dir;
	new_dir = (DirEntry_t *)&Clusters[id - 2];
	add_entry(new_dir, ".          ", ATTR_DIRECTORY, id, 0);
	add_entry(new_dir, "..         ", ATTR_DIRECTORY, parent_cluster_nb, 0);
	return (new_dir);
}

int add_entry(DirEntry_t *dir, char *name, uint8_t atttributes, uint32_t cluster_nb, uint32_t size)
{
	int id = 0;
	while (dir[id].Name[0] != 0x00)
	{
		id++;
		if (id >= (Cluster_size / sizeof(DirEntry_t)))
		{
			printf("Directory is full!\n");
			return (-1);
		};
	};

	for (uint8_t i = 0; i < 11; i++)
	{
		dir[id].Name[i] = name[i];
	}
	dir[id].Attribute0 = atttributes;
	dir[id].cluster_nb_high = cluster_nb >> 16;
	dir[id].cluster_nb_low = cluster_nb;
	dir[id].size = size;

	return (0);
}