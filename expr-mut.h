#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

/* Expression tree node types */
typedef enum {OPERATOR, OPERAND} NodeType;

/* Enum declaring variables (operands) used in arithmetic expression */
/* Index of N_OPNDS denotes number of operands, so it must always come after the last operand
 * in the enum */
typedef enum {OPND_X0, OPND_X1, OPND_X2, OPND_X3, OPND_X4, OPND_X5, OPND_X6, OPND_X7, N_OPNDS, INVALID_OPND} OperandName;

/* Data types used in arithmetic expression */
/* Index of N_TYPES denotes number of types, so it must always come after the last type in the 
 * enum */
typedef enum {UI64, UI32, UI16, UI8, I64, I32, I16, I8, N_TYPES} OperandDataType;

/* C arithmetic operators - precedence listed as on cppreference.com */
/* Index of N_OPTRS denotes number of operators, so it must always come after the last operator in
 * the enum */
typedef enum {                     
	CAST,		            /* 2 */
	MUL, DIV, REM, 	            /* 3 */
	ADD, SUB,                   /* 4 */
	LSH, RSH,                   /* 5 */
	SML, SMLEQ, GRT, GRTEQ,     /* 6 */     
	EQ, NEQ,       		    /* 7 */ 
	AND,                        /* 8 */
	XOR,                        /* 9 */
	OR,                         /* 10 */
	ANDL,		            /* 11 */
	ORL,                        /* 12 */
	N_OPTRS,	
	INVALID_OPTR
} OperatorName;

typedef struct mutatorstate {	
	unsigned int operand_uses[N_OPNDS];
	int opnd_type_map[N_OPNDS];
} MutatorState;

typedef struct node {
	NodeType type;
	union {
		OperatorName operator_name;
		OperandName operand_name;
	};
	OperandDataType opnd_data_type;
	union {
		/* Unsigned types */
		uint64_t val_ui64;
		uint32_t val_ui32;
	       	uint16_t val_ui16;
		uint8_t val_ui8;
		/* Signed types */	
		int64_t val_i64;
		int32_t val_i32;
		int16_t val_i16;
		int8_t val_i8;	
	};	
	struct node *left;
	struct node *right;
} Node;

OperandName strToOperandName(char *str_begin, char *str_end);
OperatorName strToOperatorName(char *str_begin, char *str_end);
char *operandNameToStr(OperandName operand_name);
char *operatorNameToStr(OperatorName operator_name);
int getOperatorCharCount(OperatorName operator_name);

int getPrecedence(OperatorName operator_name);
bool isLRAssociative(int precedence);

void *randomInteger(OperandDataType type);
char *intToStr(void *value, OperandDataType type);
char *intTypeToStr(OperandDataType type);

MutatorState *newMutatorState();
void freeMutatorState(MutatorState **mutator_state);

char *removeUnmatchedParentheses(char *expr);
void trimExpr(char *expr);

Node* newOperatorNode(OperatorName operator_name, Node *left, Node *right);
Node* newOperandNode(OperandName operand_name);

Node* parseExpr(char *str_begin, char *str_end);
void freeExprTree(Node **tree);

void countOpndUses(Node *root, MutatorState *mutator_state);
void restoreUnusedOpnds(Node *root, MutatorState *state);
void randomBranchSwap(Node *tree, unsigned int splitChance, unsigned int selectChance, unsigned int swap_attempts);
void randomReplaceOptrs(Node *root);
char *exprTreeToString(Node* root, int parent_precedence, int *out_str_len);
