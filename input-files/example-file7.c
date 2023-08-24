#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
int main() {
	/* start */
	uint64_t x0 = 1000000000000000001;///
	uint32_t x1 = -6;////////////////////
	uint16_t x2 = 43439;/////////////////
	uint8_t x3 = -1;/////////////////////
	uint32_t x4 = 11234;/////////////////
	int64_t x5 = 11257656656663;/////////
	uint32_t x6 = 189043737319;//////////
	uint32_t x7 = 1123443344;////////////
	uint8_t x8 = 99;/////////////////////
	int8_t x9 = 0;///////////////////////
	int32_t x10 = -2344481;//////////////
	uint8_t x11 = 127;///////////////////
	int64_t x12 = -1234234324;///////////
	uint64_t x13 = 84885;////////////////
	uint32_t x14 = 8898911255;///////////
	int16_t x15 = 4885;//////////////////
	uint64_t result;
	/* end */
	uint32_t* LH;
	uint32_t* RH;
repeat:
	LH = ((uint32_t*)(&x0));
	RH = ((uint32_t*)(&x0) + 1);
	result = x0!=(x1+x2-(x3|x4)%x5)+x6+x7||x8>x9<(x10*x11+(x12-x13)|x14*x15);
	if (*RH <= 1000000) {
		*RH *= result;
		goto repeat;
	}
	else {
		*RH = x0;	
	}
	printf("%d", result);
}/* finish */
