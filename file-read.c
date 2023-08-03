#include "expr-mut.h"

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#define MAX_FILE 8192

typedef struct my_mutator {
	MutatorState *mutator_state;
	uint8_t *mutated_out;
} my_mutator_t;

my_mutator_t *afl_custom_init(unsigned int seed) {
	srand(seed);

	my_mutator_t *mutator = calloc(1, sizeof(my_mutator_t));
	if (!mutator) {
		perror("afl_custom_int allocation failure");
		return NULL;
	}

	if ((mutator->mutated_out = (uint8_t*)malloc(MAX_FILE)) == NULL) {
		perror("afl_custom_init memory allocation failure");
		return NULL;
	}

	mutator->mutator_state = newMutatorState();

	return mutator;
}

size_t afl_custom_fuzz(my_mutator_t *mutator, uint8_t *buf, size_t buf_size, 
		       uint8_t **out_buf, uint8_t *add_buf, 
		       size_t add_buf_size, size_t max_size) {
	
	size_t mutated_size = max_size; /*TODO: temporary size */
	
	memcpy(mutator->mutated_out, buf, buf_size);

	printf("%s\n", mutator->mutated_out);
	
	//memcpy(data->mutated_out, commands[rand() % 3], 3);
	return mutated_size;
}

int main(int argc, char** argv) {
	FILE *file = NULL;
	long length = -1;

	file = fopen("example-file.c", "r");
	if (file == NULL) {
		printf("Failed to open the file\n");	
		return 1;
	}

	if (fseek(file, 0L, SEEK_END) != 0) {
		printf("Failed to seek to the end of the file\n");
		return 1;
	}
	
	length = ftell(file);
	if (length == -1) {
		printf("Failed to tell the position in the file stream\n");
		return 1;
	}

	rewind(file);

	// Make sure to terminate with 0 (length might need to be +1)	
	char file_buf[length+1];
	fread(file_buf, sizeof(*file_buf), length, file);
	if (ferror(file) != 0) {
		printf("An error occured while reading from the file\n");
		return 1;
	}
	file_buf[length] = 0;

	uint8_t *out_buf;
	my_mutator_t *mutator = afl_custom_init(1);
	size_t out_size = afl_custom_fuzz(mutator, (uint8_t*)file_buf, 
			(size_t)length, &out_buf, 
			NULL, 0, 8192);

	fclose(file);

	return 0;
}
