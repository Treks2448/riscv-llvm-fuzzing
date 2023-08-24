#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
uint32_t com(uint64_t x0, int64_t x1, uint64_t x2, int64_t x3,
             int32_t x4, uint32_t x5, int16_t x6, uint16_t x7);
int main() {
	/* start */
	uint64_t x0 = 11111111111;///////////
	int64_t x1 = -124434448874;//////////
	uint32_t x2 = -1;////////////////////
	int32_t x3 = 22299445;///////////////
	uint16_t x4 = 68661;/////////////////
	int16_t x5 = 0;//////////////////////
	uint8_t x6 = 113;////////////////////
	int8_t x7 = -127;////////////////////
	uint64_t result;
	/* end */
	uint32_t out = com(x0, x1, x2, x3, x4, x5, x6, x7);
	printf("%d", out);
}
uint32_t com(uint64_t x0, int64_t x1, uint64_t x2, int64_t x3,
	     int32_t x4, uint32_t x5, int16_t x6, uint16_t x7) {
	int8_t x8 = rand();//////////////////
	int16_t x9 = rand();/////////////////
	int32_t x10 = rand();////////////////
	int64_t x11 = rand();////////////////
	uint8_t x12 = rand();////////////////
	uint64_t x13 = rand();///////////////
	uint32_t x14 = rand();///////////////
	uint16_t x15 = rand();///////////////
	uint32_t result = x0+x1+x2|(x3*x4%x5)^x6^x7*x8|(x9+(x10%x11))+(x12==x13)*x14&x15;
	return result;
}
/* finish */
