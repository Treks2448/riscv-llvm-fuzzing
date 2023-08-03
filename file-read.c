#include "expr-mut.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
	
	/* TODO: allocating max file size makes it harder to insert data into the buffer */
	if ((mutator->mutated_out = (uint8_t*)malloc(MAX_FILE)) == NULL) {
		perror("afl_custom_init memory allocation failure");
		return NULL;
	}

	mutator->mutator_state = newMutatorState();

	return mutator;
}

/* Inserts data into the buffer. Data following insertion is displaced 
 * by size of inserted data. The buffer size is increased accordingly. */
void insertData(uint8_t **buf, size_t *buf_size, size_t insert_pos
		 uint8_t *new_data, size_t new_data_size) {
	/* Increase buffer size to accomodate for inserted data */
	*buf_size += new_data_size;
	*buf = (uint8_t)realloc(*buf, *buf_size);
	if (*buf == NULL) {
		printf("data insertion failure");
		return;
	}
	
	/* Make space for inserted data */
	memmove(*buf + insert_pos + new_data_size, 
		*buf + insert_pos,
		*buf_size - insert_pos - new_data_size);

	/* Insert data into buffer */	
	memcpy(*buf + insert_pos, new_data, new_data_size);
}

/* Finds string in buffer and returns its location */
int findString(uint8_t *buf, size_t buf_size, const char *search_str) {
	size_t search_str_len = strlen(search_str);	
	for (size_t i = 0; i <= buf_size - search_str_len; i++)
		if (memcmp(buf + i, search_str, search_str_len) == 0)
			return (int)i;
	return -1;
}

/* Deletes specified number of bytes from buffer at start location */
void deleteFromBuf(uint8_t *buf, size_t buf_size, size_t start, size_t n_bytes) {
	size_t n_to_del = start + n_bytes > buf_size ? buf_size - start : n_bytes;
	uint8_t *src = buf + start + n_to_del;
	uint8_t *dest = buf + start;
	size_t n_shift = buf_size - start - n_to_del;
	memmove(dest, src, n_shift);
}

size_t afl_custom_fuzz(my_mutator_t *mutator, uint8_t *buf, size_t buf_size, 
		       uint8_t **out_buf, uint8_t *add_buf, 
		       size_t add_buf_size, size_t max_size) {
	
	size_t mutated_size = max_size; /*TODO: temporary size */
	
	memcpy(mutator->mutated_out, buf, buf_size);

	/* find each variable and its type in the file (buffer) */

	/* for each found variable
	 * 	set its type
	 * 	generate and set its value
	 * 	write new type and value to the buffer (as hex ideally)
	 */

	/* find the result variable and set its type (write to file) */

	/* find the beginning of the expression in the file */
	
	/* extract the expression as a string */

	/* process expression using expr-mut */
	/* overwrite the expression with the processed expression */

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
