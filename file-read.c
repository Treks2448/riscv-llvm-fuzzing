#include "expr-mut.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef uint8_t u8;
typedef uint8_t afl_state_t;

typedef struct my_mutator {
	afl_state_t *afl;
	
	size_t trim_size_current;
	int trimming_steps;
	int cur_step;

	u8 *post_process_buf;
	u8 *trim_buf;

	MutatorState *mutator_state;

	u8 *mutated_out;
	long unsigned int mutated_out_size;
} my_mutator_t;

void insertData(u8 **buf, size_t *buf_size, size_t insert_pos,
		u8 *new_data, size_t new_data_size);
int findString(u8 *buf, size_t buf_size, const char *search_str, size_t start_pos);
void deleteFromBuf(u8 *buf, size_t buf_size, size_t start, size_t n_bytes);

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

size_t afl_custom_fuzz(my_mutator_t *mutator, u8 *buf, size_t buf_size,
		       u8 **out_buf, u8 *add_buf, 
		       size_t add_buf_size, size_t max_size) {
		
	/*TODO: double check that correct number of bytes are allocated */	
	mutator->mutated_out = (u8*)malloc(buf_size + 1);
	mutator->mutated_out_size = buf_size;
	memcpy(mutator->mutated_out, buf, buf_size);	
	
	/* Remove integer declarations from file */	
	int start_loc = findString(mutator->mutated_out, 
				   mutator->mutated_out_size,
				   "/* start */\n", 
				   0);
	int end_loc = findString(mutator->mutated_out,
				 mutator->mutated_out_size,
				 "/* end */", 
				 start_loc);
	if (start_loc == -1 || end_loc == -1) {
		printf("Could not find start/end location in file buffer");
		exit(1);
	}
	deleteFromBuf(mutator->mutated_out, mutator->mutated_out_size, 
		      start_loc + 12, end_loc - (start_loc + 12));

	/* generate and insert integer declarations and values into file */
	int cursor = start_loc + 12;
	int max_type_strlen = 8; /* uint64_t */
	int max_line_strlen = 39; /* "\tuint64_t x0 = 11110000111100001111;//\n" */
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
		/* "\ttype name = value;//\n" */		
		char *line = calloc(max_line_strlen + 1, sizeof(char));
		strcat(line, "\t");
		strcat(line, type_str);
		for (int i = 0, n = max_type_strlen - strlen(type_str) + 1; i < n; i++)
			strcat(line, " ");
		strcat(line, opnd_name_str);
		strcat(line, " = ");
		strcat(line, value_str);
		strcat(line, ";//");	
		for (int i = 0, n = max_line_strlen - strlen(line) - 1; i < n; i++)
			strcat(line, "/");
		strcat(line, "\n");
		
		/* Insert string into file buffer */	
		insertData(&mutator->mutated_out, &mutator->mutated_out_size,
			   (size_t)cursor, (u8*)line, (size_t)max_line_strlen);

		cursor += max_line_strlen; 

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
		   (size_t)cursor, (u8*)line, (size_t)strlen(line));
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
				  expr_begin);
	if (expr_begin == -1 || expr_end == -1) {
		printf("Could not find end/start location in file buffer");
		exit(1);
	}

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
		   (size_t)expr_begin, (u8*)mutated_expr, (size_t)mutated_expr_strln);

	free(mutated_expr);
	free(cleaned_expr);
	freeExprTree(&tree);
	freeMutatorState(&mutator->mutator_state);
	
	if (max_size < mutator->mutated_out_size) {
		printf("The mutated output is larger than the max size");
		exit(1);
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

	u8 *out_buf;
	my_mutator_t *mutator = afl_custom_init(NULL,1);
	size_t out_size = afl_custom_fuzz(mutator, (u8*)file_buf, 
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
void insertData(u8 **buf, size_t *buf_size, size_t insert_pos,
		u8 *new_data, size_t new_data_size) {
	/* Increase buffer size to accomodate for inserted data */
	*buf_size += new_data_size;
	if (insert_pos > *buf_size) {
		printf("insert position outside of buffer in insertData");
		return;
	}
	*buf = (u8*)realloc(*buf, *buf_size);
	if (*buf == NULL) {
		printf("realloc failure in insertData");
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
int findString(u8 *buf, size_t buf_size, const char *search_str, size_t start_pos) {
	if (start_pos >= buf_size) {
		printf("start position outside of buffer in findString");
		return -1;
	}
	size_t search_str_len = strlen(search_str);	
	for (size_t i = start_pos; i <= buf_size - search_str_len; i++)
		if (memcmp(buf + i, search_str, search_str_len) == 0)
			return (int)i;
	return -1;
}

/* Deletes specified number of bytes from buffer at start location */
void deleteFromBuf(u8 *buf, size_t buf_size, size_t start, size_t n_bytes) {
	size_t n_to_del = start + n_bytes > buf_size ? buf_size - start : n_bytes;
	u8 *src = buf + start + n_to_del;
	u8 *dest = buf + start;
	size_t n_shift = buf_size - start - n_to_del;
	memmove(dest, src, n_shift);
}


