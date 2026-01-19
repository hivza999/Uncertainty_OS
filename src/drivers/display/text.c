#include <stdint.h>

#define VIDEO_MEMORY 0xb8000
#define Scren_Width 80
#define Scren_Height 25

void print_size_o(uint64_t value, uint8_t color);
void print_d10(uint32_t value, uint8_t color);
void echo(char value, uint8_t color);
void print(char *string, uint8_t color);
void hexprint64(uint64_t value, uint8_t color);
void hexprint32(uint32_t value, uint8_t color);
void hexprint16(uint16_t value, uint8_t color);
void hexprint8(uint8_t value, uint8_t color);
void hexprint(uint8_t digit, uint8_t color);
void scroll(uint8_t lines);

static uint32_t Cursor = VIDEO_MEMORY;

void print_size_o(uint64_t value, uint8_t color)
{
	if (value < (10000))
	{
		print_d10(value, color);
		print(" B", color);
	}
	else if (value < (10000 * 1024))
	{
		print_d10(value / 1024, color);
		print(" KiB", color);
	}
	else if (value < ((uint64_t)10000 * 1024 * 1024))
	{
		print_d10(value / (1024 * 1024), color);
		print(" MiB", color);
	}
	else if (value < ((uint64_t)10000 * 1024 * 1024 * 1024))
	{
		print_d10(value / ((uint64_t)1024 * 1024 * 1024), color);
		print(" GiB", color);
	}
	else
	{
		print_d10(value / ((uint64_t)1024 * 1024 * 1024 * 1024), color);
		print(" TiB", color);
	}
}

void print_d10(uint32_t value, uint8_t color)
{
	char buffer[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	char i = 10;
	do
	{
		i--;
		buffer[i] = value % 10 + '0';
		value = value / 10;
	} while (value > 0);

	print(buffer + i, color);
}

void hexprint64(uint64_t value, uint8_t color)
{
	hexprint(value >> 60, color);
	hexprint((value >> 56) & 0x0f, color);
	hexprint((value >> 52) & 0x0f, color);
	hexprint((value >> 48) & 0x0f, color);
	hexprint((value >> 44) & 0x0f, color);
	hexprint((value >> 40) & 0x0f, color);
	hexprint((value >> 36) & 0x0f, color);
	hexprint((value >> 32) & 0x0f, color);
	hexprint((value >> 28) & 0x0f, color);
	hexprint((value >> 24) & 0x0f, color);
	hexprint((value >> 20) & 0x0f, color);
	hexprint((value >> 16) & 0x0f, color);
	hexprint((value >> 12) & 0x0f, color);
	hexprint((value >> 8) & 0x0f, color);
	hexprint((value >> 4) & 0x0f, color);
	hexprint(value & 0x0f, color);
}

void hexprint32(uint32_t value, uint8_t color)
{
	hexprint(value >> 28, color);
	hexprint((value >> 24) & 0x0f, color);
	hexprint((value >> 20) & 0x0f, color);
	hexprint((value >> 16) & 0x0f, color);
	hexprint((value >> 12) & 0x0f, color);
	hexprint((value >> 8) & 0x0f, color);
	hexprint((value >> 4) & 0x0f, color);
	hexprint(value & 0x0f, color);
}

void hexprint16(uint16_t value, uint8_t color)
{
	hexprint(value >> 12, color);
	hexprint((value >> 8) & 0x0f, color);
	hexprint((value >> 4) & 0x0f, color);
	hexprint(value & 0x0f, color);
}

void hexprint8(uint8_t value, uint8_t color)
{
	hexprint(value >> 4, color);
	hexprint(value & 0x0f, color);
}

void hexprint(uint8_t digit, uint8_t color)
{
	if (digit < 10)
	{
		*(char *)(Cursor) = digit + '0';
		*(uint8_t *)(Cursor + 1) = color;
	}
	else
	{
		*(char *)(Cursor) = digit + ('a' - 10);
		*(uint8_t *)(Cursor + 1) = color;
	}
	Cursor += 2;
}

void print(char *string, uint8_t color)
{
	while (string[0])
	{
		echo(string[0], color);
		string++;
	}
}

void echo(char value, uint8_t color)
{
	*(char *)(Cursor + 1) = 0x0f;

	switch (value)
	{
	case ('\n'):
		Cursor += Scren_Width * 2 - ((Cursor - VIDEO_MEMORY) % (Scren_Width * 2));
		break;

	case ('\r'):
		Cursor -= ((Cursor - VIDEO_MEMORY) % (Scren_Width * 2));
		break;

	case ('\b'):
		Cursor -= 2;
		*(char *)(Cursor) = 0x00;
		break;

	case (0x1b): // escape
		Cursor = VIDEO_MEMORY;
		scroll(Scren_Height);
		break;

	default:
		*(char *)(Cursor) = value;
		*(uint8_t *)(Cursor + 1) = color;
		Cursor += 2;
	}

	if (Cursor >= VIDEO_MEMORY + Scren_Height * Scren_Width * 2)
	{
		Cursor -= Scren_Width * 2;
		scroll(1);
	}

	*(char *)(Cursor + 1) = 0xf0;
}

void scroll(uint8_t lines)
{
	uint32_t i;
	for (i = VIDEO_MEMORY; i < VIDEO_MEMORY + (Scren_Width * (Scren_Height - lines) * 2); i++)
	{
		*(uint8_t *)i = *(uint8_t *)(i + Scren_Width * lines * 2);
	}
	while (i < VIDEO_MEMORY + (Scren_Width * Scren_Height * 2))
	{
		*(uint8_t *)i = 0;
		i++;
	}
}