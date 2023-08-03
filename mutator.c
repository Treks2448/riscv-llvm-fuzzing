#include "afl-fuzz.h"
#include "expr-mut.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct my_mutator {
	afl_state_t *afl;
	
	MutatorState *mutator_state;
	u8 *mutated_out;
} my_mutator_t;

my_mutator_t *afl_custom_init(afl_state_t *afl, unsigned int seed) {
	srand(seed);

	my_mutator_t *mutator = calloc(1, sizeof(my_mutator_t));
	if (!mutator) {
		perror("afl_custom_int allocation failure");
		return NULL;
	}

	if ((mutator->mutated_out = (u8*)malloc(MAX_FILE)) == NULL) {
		perror("afl_custom_init memory allocation failure");
		return NULL;
	}

	mutator->afl = afl;

	return mutator;
}

size_t afl_custom_fuzz(my_mutator_t *data, uint8_t *buf, size_t buf_size, 
		       u8 **out_buf, uint8_t *add_buf, 
		       size_t add_buf_size, size_t max_size) {
	//size_t mutated_size = DATA_SIZE <= max_size ? DATA_SIZE : max_size;

	//memcpy(data->mutated_out, buf, buf_size);

	//memcpy(data->mutated_out, commands[rand() % 3], 3);
}


