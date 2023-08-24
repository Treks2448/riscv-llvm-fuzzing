#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
void foo(uint32_t* a);
int main() {
	uint64_t final = 0;	
	for (int i = 0; i < 128; i++) {
		/* start */
		uint64_t x0 = 1281894565489692596;///
		uint64_t x1 = 8096;//////////////////
		uint32_t x2 = -1;////////////////////
		int32_t x3 = 98527265;///////////////
		uint16_t x4 = 11001;/////////////////
		int16_t x5 = 12348;//////////////////
		uint64_t x6 = 1555559999999999;//////
		uint64_t x7 = 0;/////////////////////
		uint8_t x8 = 112;////////////////////
		int8_t x9 = -1;//////////////////////
		int32_t x10 = -5573991;//////////////
		uint32_t x11 = 0;////////////////////
		uint16_t x12 = 0;////////////////////
		uint64_t x13 = 978150205;////////////
		uint8_t x14 = 255;///////////////////
		int8_t x15 = -45;////////////////////
		uint64_t result;
		/* end */
		x0 = x0 & (x1 + x2 * x3 + x4);
		foo((uint32_t*)&x0);
		result = x0|x1&x2!=x3-x4>x5<x6>>x7<<x8/(x9==(x10>>x11))+x12/x13==(x14+x15);
		foo((uint32_t*)&result);
		final = result;
	}	
	printf("%d", final);
}/* finish */

void foo(uint32_t* a) {*a = *a & 0x0000FFFF;}
