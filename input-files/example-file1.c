#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
int main() {
	/* start */
	uint32_t x0 = 1134213411111;/////////
	int64_t x1 = -1234444;///////////////
	uint8_t x2 = 0;//////////////////////
	int16_t x3 = 9;//////////////////////
	uint64_t x4 = 555991;////////////////
	int64_t x5 = -18;////////////////////
	uint64_t x6 = 65656565;//////////////
	uint64_t x7 = 18883335;//////////////
	uint8_t x8 = 998912;/////////////////
	int8_t x9 = -1;//////////////////////
	int32_t x10 = -3888881;//////////////
	uint8_t x11 = 183;///////////////////
	int64_t x12 = 0;/////////////////////
	uint64_t x13 = 8488150448205;////////
	uint64_t x14 = 98898911255;//////////
	int32_t x15 = -48885;////////////////
	uint64_t result;
	/* end */
	x0 = x0 & 0x00000000ffffffff;////////
	x1 = x1 & 0x0000ffff;////////////////
	x2 = x2 & 0x00ff;////////////////////
	x3 = x3 & 0x0f;//////////////////////
	result = x0<<(x1*x2)|((x3&x4)%x5)&x6+x7*(x8|x9+(x10||x11)+(x12>x13))*(x14&x15);
	printf("%d", result);
}/* finish */
