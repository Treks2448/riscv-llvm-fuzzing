#include "expr-mut.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct my_mutator {
	afl_state_t *afl;
	
	size_t trim_size_current;
	int trimming_steps;
	int cur_step;

	MutatorState *mutator_state;

	uint8_t *mutated_out;
	long unsigned int mutated_out_size;
} my_mutator_t;

void insertData(uint8_t **buf, size_t *buf_size, size_t insert_pos,
		uint8_t *new_data, size_t new_data_size);
int findString(uint8_t *buf, size_t buf_size, const char *search_str, size_t start_pos);
void deleteFromBuf(uint8_t *buf, size_t buf_size, size_t start, size_t n_bytes);

my_mutator_t *afl_custom_init(afl_state_t *afl, unsigned int seed) {
	srand(seed);

	my_mutator_t *mutator = calloc(1, sizeof(my_mutator_t));
	if (!mutator) {
		perror("afl_custom_int allocation failure");
		return NULL;
	}
	
	mutator->post_process_buf = NULL;
	mutator->trim_buf = NULL;
	mutator->mutated_out = NULL;

	mutator->mutator_state = newMutatorState();
	
	mutator->afl = afl;

	return mutator;
}

size_t afl_custom_fuzz(my_mutator_t *mutator, uint8_t *buf, size_t buf_size,
		       uint8_t **out_buf, uint8_t *add_buf, 
		       size_t add_buf_size, size_t max_size) {
		
	/*TODO: double check that correct number of bytes are allocated */	
	mutator->mutated_out = (uint8_t*)malloc(buf_size + 1);
	mutator->mutated_out_size = buf_size;
	memcpy(mutator->mutated_out, buf, buf_size);	
	
	/* Remove integer declarations from file */	
	int start_loc = findString(mutator->mutated_out, 
				   mutator->mutated_out_size,
				   "/* start */\n", 
				   0);
	int end_loc = findString(mutator->mutated_out,
				 mutator->mutated_out_size,
				 "/* end */\n", 
				 start_loc);
	deleteFromBuf(mutator->mutated_out, mutator->mutated_out_size, 
		      start_loc + 12, end_loc - (start_loc + 12));

	/* generate and insert integer declarations and values into file */
	int cursor = start_loc + 12;	
	for (int i = 0; i < N_OPNDS; i++) {
		/* Get variable name as string */	
		char *opnd_name_str = operandNameToStr((OperandName)i);

		/* Generate variable value and its string representation */	
		int type = mutator->mutator_state->opnd_type_map[i];
		void *value = randomInteger((OperandDataType)type);
		char *value_str = intToStr(value, (OperandDataType)type);

		/* Get variable type name as string */
		char *type_str = intTypeToStr(type);

		/* Build string declaring variable and setting its value */
		/* "\ttype name = value;\n" */	
		int line_strlen = 1 + 
			   strlen(type_str) + 1 +       
			   strlen(opnd_name_str) + 3 +  
			   strlen(value_str) + 2;
		char *line = calloc(line_strlen + 1, sizeof(char));
		strcat(line, "\t");
		strcat(line, type_str);
		strcat(line, " ");
		strcat(line, opnd_name_str);
		strcat(line, " = ");
		strcat(line, value_str);
		strcat(line, ";\n");
		
		/* Insert string into file buffer */	
		insertData(&mutator->mutated_out, &mutator->mutated_out_size,
			   (size_t)cursor, (uint8_t*)line, (size_t)line_strlen);

		cursor += line_strlen; 

		free(value);
		free(value_str);
		free(line);
	}
	
	/* Create the result variable and its type */
	char *type_str = intTypeToStr((OperandDataType)(rand() % N_TYPES));
	char *line = calloc(strlen(type_str) + 12, sizeof(char));
	strcat(line, "\t");
	strcat(line, type_str);
	strcat(line, " ");
	strcat(line, "result;\n\t");
	insertData(&mutator->mutated_out, &mutator->mutated_out_size, 
		   (size_t)cursor, (uint8_t*)line, (size_t)strlen(line));
	cursor += strlen(line);
	free(line);	

	/* Find the beginning of the expression in the file */
	int expr_begin = 9 + findString(mutator->mutated_out, 
					mutator->mutated_out_size, 
					"result", 
					cursor);
	int expr_end = findString(mutator->mutated_out, 
				  mutator->mutated_out_size, 
				  ";", 
				  cursor);

	/* Extract the expression as a string */
	char *expr_str = calloc(expr_end - expr_begin + 1, sizeof(char));
	memcpy(expr_str, mutator->mutated_out + expr_begin, expr_end - expr_begin);
	trimExpr(expr_str);
	char *cleaned_expr = removeUnmatchedParentheses(expr_str);
	free(expr_str);

	/* Parse and mutate the expression */	
	Node *tree = parseExpr(cleaned_expr, cleaned_expr + strlen(cleaned_expr));
	countOpndUses(tree, mutator->mutator_state);
	restoreUnusedOpnds(tree, mutator->mutator_state);
	randomBranchSwap(tree, 40, 50, 3);
	randomReplaceOptrs(tree);
	
	/* Get the mutated expression as a string and write to buffer */
	int mutated_expr_strln;
	char *mutated_expr = exprTreeToString(tree, 16, &mutated_expr_strln);
	
	/* Overwrite the expression with the processed expression */
	deleteFromBuf(mutator->mutated_out, mutator->mutated_out_size, 
		      expr_begin , expr_end - expr_begin);
	insertData(&mutator->mutated_out, &mutator->mutated_out_size, 
		   (size_t)expr_begin, (uint8_t*)mutated_expr, (size_t)mutated_expr_strln);

	free(mutated_expr);
	free(cleaned_expr);
	freeExprTree(&tree);
	freeMutatorState(&mutator->mutator_state);
	
	if (max_size < mutator->mutated_out_size) {
		printf("The mutated output is larger than the max size");
		exit(0);
	}
	printf("%s\n", mutator->mutated_out);	
	
	*out_buf = mutator->mutated_out;	
	return mutator->mutated_out_size;
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
			(size_t)(length+1), &out_buf, 
			NULL, 0, 8192);

	fclose(file);

	return 0;
}

void afl_custom_deinit(my_mutator_t *mutator) {
	free(mutator->mutated_out);
	free(mutator);
}

/* Inserts data into the buffer. Data following insertion is displaced 
 * by size of inserted data. The buffer size is increased accordingly. */
void insertData(uint8_t **buf, size_t *buf_size, size_t insert_pos,
		uint8_t *new_data, size_t new_data_size) {
	/* Increase buffer size to accomodate for inserted data */
	*buf_size += new_data_size;
	*buf = (uint8_t*)realloc(*buf, *buf_size);
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
int findString(uint8_t *buf, size_t buf_size, const char *search_str, size_t start_pos) {
	size_t search_str_len = strlen(search_str);	
	for (size_t i = start_pos; i <= buf_size - search_str_len; i++)
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


