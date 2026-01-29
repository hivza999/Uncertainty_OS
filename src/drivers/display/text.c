#include <stdint.h>
#include "text.h"

#define VIDEO_MEMORY 0xb8000
#define Scren_Width 80
#define Scren_Height 25

static uint32_t Cursor = VIDEO_MEMORY;
static uint8_t Cursor_color = 0x0f;

void print_size_B(uint64_t value)
{
	if (value < (10000))
	{
		print_d10(value);
		print(" B");
	}
	else if (value < (10000 * 1024))
	{
		print_d10(value / 1024);
		print(" KiB");
	}
	else if (value < ((uint64_t)10000 * 1024 * 1024))
	{
		print_d10(value / (1024 * 1024));
		print(" MiB");
	}
	else if (value < ((uint64_t)10000 * 1024 * 1024 * 1024))
	{
		print_d10(value / ((uint64_t)1024 * 1024 * 1024));
		print(" GiB");
	}
	else
	{
		print_d10(value / ((uint64_t)1024 * 1024 * 1024 * 1024));
		print(" TiB");
	}
}

void print_d10(uint32_t value)
{
	char buffer[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	char i = 10;
	do
	{
		i--;
		buffer[i] = value % 10 + '0';
		value = value / 10;
	} while (value > 0);

	print(buffer + i);
}

void hexprint64(uint64_t value)
{
	hexprint(value >> 60);
	hexprint((value >> 56) & 0x0f);
	hexprint((value >> 52) & 0x0f);
	hexprint((value >> 48) & 0x0f);
	hexprint((value >> 44) & 0x0f);
	hexprint((value >> 40) & 0x0f);
	hexprint((value >> 36) & 0x0f);
	hexprint((value >> 32) & 0x0f);
	hexprint((value >> 28) & 0x0f);
	hexprint((value >> 24) & 0x0f);
	hexprint((value >> 20) & 0x0f);
	hexprint((value >> 16) & 0x0f);
	hexprint((value >> 12) & 0x0f);
	hexprint((value >> 8) & 0x0f);
	hexprint((value >> 4) & 0x0f);
	hexprint(value & 0x0f);
}

void hexprint32(uint32_t value)
{
	hexprint(value >> 28);
	hexprint((value >> 24) & 0x0f);
	hexprint((value >> 20) & 0x0f);
	hexprint((value >> 16) & 0x0f);
	hexprint((value >> 12) & 0x0f);
	hexprint((value >> 8) & 0x0f);
	hexprint((value >> 4) & 0x0f);
	hexprint(value & 0x0f);
}

void hexprint16(uint16_t value)
{
	hexprint(value >> 12);
	hexprint((value >> 8) & 0x0f);
	hexprint((value >> 4) & 0x0f);
	hexprint(value & 0x0f);
}

void hexprint8(uint8_t value)
{
	hexprint(value >> 4);
	hexprint(value & 0x0f);
}

void hexprint(uint8_t digit)
{
	if (digit < 10)
	{
		echo(digit + '0');
	}
	else
	{
		echo(digit + ('a' - 10));
	}
}

void print(char *string)
{
	while (string[0])
	{
		echo(string[0]);
		string++;
	}
}

void echo(char value)
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
		screen_scroll(Scren_Height);
		break;

	default:
		*(char *)(Cursor) = value;
		*(uint8_t *)(Cursor + 1) = Cursor_color;
		Cursor += 2;
	}

	if (Cursor >= VIDEO_MEMORY + Scren_Height * Scren_Width * 2)
	{
		Cursor -= Scren_Width * 2;
		screen_scroll(1);
	}

	*(char *)(Cursor + 1) = 0xf0;
}

void screen_scroll(uint8_t lines)
{
	uint32_t i;
	for (i = VIDEO_MEMORY; i < VIDEO_MEMORY + (Scren_Width * (Scren_Height - lines) * 2); i++)
	{
		*(uint8_t *)i = *(uint8_t *)(i + Scren_Width * lines * 2);
	}
	while (i < VIDEO_MEMORY + (Scren_Width * Scren_Height * 2))
	{
		*(uint8_t *)i = 0;
		*(uint8_t *)(i + 1) = Cursor_color;
		i += 2;
	}
}

void cursor_color(uint8_t color)
{
	Cursor_color = color;
}