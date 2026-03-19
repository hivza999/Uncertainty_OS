#include <stdint.h>

void main()
{
	for (uint32_t i = 0; i < 80 * 25 * 2; i += 2)
	{
		*(char *)(0xb8000 + i) = (char)i;
		*(uint8_t *)(0xb8001 + i) = (uint8_t)i;
	}
}
