#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
uint16_t com(uint32_t x0, int32_t x1, uint32_t x2, int32_t x3,
             int16_t x4, uint16_t x5, int8_t x6, uint64_t x7);
int main() {
	/* start */
	uint8_t x0 = 0;//////////////////////
	uint16_t x1 = 144;///////////////////
	uint16_t x2 = -1234;/////////////////
	uint32_t x3 = 1673451;///////////////
	uint64_t x4 = 1065278969578;/////////
	int64_t x5 = -4484881;///////////////
	int32_t x6 = 90825;//////////////////
	uint32_t x7 = 1;/////////////////////
	uint64_t result;
	/* end */
	uint16_t out = com(x0, x1, x2, x3, x4, x5, x6, x7);	
	printf("%d", out);
}
uint16_t com(uint32_t x0, int32_t x1, uint32_t x2, int32_t x3,
             int16_t x4, uint16_t x5, int8_t x6, uint64_t x7) {
	uint16_t x8 = x0 ^ x1;///////////////
	uint32_t x9 = x2 ^ x1;///////////////
	int16_t x10 = x3 & x1;///////////////
	int32_t x11 = x8 % x10;//////////////
	uint32_t x12 = x4 * x5;//////////////
	uint32_t x13 = x7 & 0x0000FFFF;//////
	uint16_t x14 = x6 + 256;/////////////
	uint64_t x15 = x0 + x13;/////////////
	int16_t result = (x0>>x1+x2)>>((x3/x4/x5)^x6^x7)+x8<<(x9|(x10%x11))>=(x12!=x13)*x14/x15;
	return result;
}
/* finish */
