#include "expr-mut.h"

MutatorState *newMutatorState() {
	MutatorState *mutator_state = malloc(sizeof(MutatorState));
	for (int i = 0; i < N_OPNDS; i++) {
		mutator_state->operand_uses[i] = 0;
		mutator_state->opnd_type_map[i] = rand() % N_TYPES;
	}

	return mutator_state;
}

void freeMutatorState(MutatorState **mutator_state) {
	free(*mutator_state);
	*mutator_state = NULL;
}

/* On input of arithmetic expression string returns new string with unmatched parentheses removed.
 * Parentheses are removed left-to-right until number of left and right brackets is the same.
 * Returned string is dynamically allocated and must be freed. */
char *removeUnmatchedParentheses(char *expr) {
	char *out_str = (char*)malloc(sizeof(char) * (strlen(expr) + 1));	
	int bracket_depth = 0;
	int out_str_i = 0;

	/* Count total number of unmatched parentheses. Total is negative for right bracket */
	for (int expr_i = 0; expr_i < strlen(expr); expr_i++) {
		if (expr[expr_i] == '(')
			bracket_depth++;
		else if (expr[expr_i] == ')')
			bracket_depth--;
	}
	
	/* On input of correctly parenthesized expression, return copy of input string */
	if (bracket_depth == 0) {
		strcpy(out_str, expr);
		out_str[strlen(expr)] = 0;
		return out_str;
	}

	/* Copy over characters, except for offending brackets, into output string */
	for (int expr_i = 0; expr_i < strlen(expr); expr_i++) {	
		if (bracket_depth > 0 && expr[expr_i] == '(') {
			bracket_depth--;
			continue;
		}
		else if (bracket_depth < 0 && expr[expr_i] == ')') {
			bracket_depth++;
			continue;
		}
		else {
			out_str[out_str_i] = expr[expr_i];
			out_str_i++;
		}	
	}
	out_str[out_str_i] = 0;
	return out_str;
}

/* Cuts off portion of expression to reduce number of variables to N_OPNDS.
 * Shortens the input string so values like string length must be recalculated. */
void trimExpr(char *expr) {
	char *str_it = expr, *cutoff_point = NULL, *str_end = expr + strlen(expr);
	int opnd_count = 0;
	while (str_it < str_end) {
		OperandName opnd_name = strToOperandName(str_it, str_end);
		if (opnd_name != INVALID_OPND) {
			opnd_count++;
			cutoff_point = str_it + 2;
			if (opnd_count == N_OPNDS)
				break;
		}
		str_it++;	
	}
	if (cutoff_point != NULL && cutoff_point < str_end) {
		*cutoff_point = '\0';
	}
}

Node* newOperatorNode(OperatorName operator_name, Node *left, Node *right) {
	Node *node = malloc(sizeof(Node));
	node->type = OPERATOR;
	node->operator_name = operator_name;
	node->left = left;
	node->right = right;
	return node;
}

Node* newOperandNode(OperandName operand_name) {
	Node *node = malloc(sizeof(Node));
	node->type = OPERAND;
	node->operand_name = operand_name;
	node->left = NULL;
	node->right = NULL;
	return node;
}

/* Builds binary arithmetic tree from C arithmetic expression string 
 * Expects operands to be variables in form xI where I is an integer.
 * Supported number of operands currently limited to 8, i.e. x0-x7 */
Node* parseExpr(char *str_begin, char *str_end) {	
	OperatorName operator_name = INVALID_OPTR; 
	OperandName operand_name = INVALID_OPND;
	int lowest_precedence = 0; /* higher value -> lower precedence */
	int bracket_depth = 0;
	char *lowest_prec_optr_loc = NULL;
	Node *left_child = NULL;
	Node *right_child = NULL;

	/* Return null node if input is empty substring */
	if (str_end - str_begin <= 0) {
		printf("No expression to parse from string of size %d", (int)(str_end - str_begin));
		return NULL;
	}	

	/* Read sub-string left-to-right, tracking lowest precedence operator */
	for (char *str_it = str_begin; str_it != str_end; str_it++) {
		/* Skip over anything that's parenthesized */	
		if (*str_it == '(')
			bracket_depth++;	
		if (*str_it == ')')
			bracket_depth--;
		if (bracket_depth > 0)
			continue;
		
		/* Try to match current section of string with an operator */
		OperatorName next_operator_name = strToOperatorName(str_it, str_end);
		if (next_operator_name == INVALID_OPTR)
			continue;
		
		/* Update lowest precedence operator if new one found */
		int next_operator_precedence = getPrecedence(next_operator_name);
		if (next_operator_precedence >= lowest_precedence) { /* LR associativity only */
			lowest_precedence = next_operator_precedence;
			operator_name = next_operator_name;
			lowest_prec_optr_loc = str_it;
		}
	
		/* Skip over remaining characters comprising operator so that part of it doesn't
		 * get read again as a different operator (e.g. = in >=). For loop iteration skips
		 * an additional character hence the - 1 */
		str_it += getOperatorCharCount(next_operator_name) - 1;
	}

	/* Expression is completely parenthesized; parse within parentheses */	
	if (operator_name == INVALID_OPTR && *str_begin == '(' && *(str_end - 1) == ')')
		return parseExpr(str_begin+1, str_end-1);

	/* If operator found, build node and parse left and right substrings for children */
	if (operator_name != INVALID_OPTR) {
		left_child = parseExpr(str_begin, lowest_prec_optr_loc);
		right_child = parseExpr(lowest_prec_optr_loc + getOperatorCharCount(operator_name), str_end);
		return newOperatorNode(operator_name, left_child, right_child);
	}
	/* No operator found so substring must contain an operand */
	else {
		operand_name = strToOperandName(str_begin, str_end);
		if (operand_name != INVALID_OPND) {
			return newOperandNode(operand_name);
		}
	}
	
	/* Nothing found, string isn't empty; print error */
	printf("Could not parse operator or operand from substring: %s", str_begin);
	return NULL;	
}

/* Deallocates memory used by expression tree */
void freeExprTree(Node **tree){
	/* Helper function to recursively free nodes of tree */
	void freeNode(Node *root) {
		/* Operator nodes have children which must be freed */	
		if (root->left->type == OPERATOR)
			freeNode(root->left);	
		if (root->right->type == OPERATOR)
			freeNode(root->right);
		/* free the nodes themselves */	
		free(root->left);
		free(root->right);
	}
	
	/* Recursively free tree and set root to NULL */
	freeNode(*tree);
	free(*tree);
	*tree = NULL;	
}

void countOpndUses(Node *root, MutatorState *mutator_state) {
	if (root == NULL) {
		printf("Encountered NULL node when counting operand uses, which is unexpected");	
		return;
	}
	else if (root->type == OPERAND) {
		mutator_state->operand_uses[root->operand_name]++;
	}
	else {
		countOpndUses(root->left, mutator_state);
		countOpndUses(root->right, mutator_state);
	}
}

void restoreUnusedOpnds(Node *root, MutatorState *state) {	
	void stackOverusedOpnds(Node *root, MutatorState *state, Node **stack, int *head) {
		if (root == NULL)
			return;
		else if (root->type == OPERAND && state->operand_uses[root->operand_name] > 1) {
			stack[*head] = root;
			state->operand_uses[stack[*head]->operand_name]--;
			(*head)++;
		}
		else {
			stackOverusedOpnds(root->left, state, stack, head);
			stackOverusedOpnds(root->right, state, stack, head);
		}
	}

	void stackUnusedOpnds(MutatorState *state, Node **stack, int *head) {
		for (int i = 0; i < N_OPNDS; i++) {
			if (state->operand_uses[i] == 0) {
				stack[*head] = newOperandNode((OperandName)i);
				(*head)++;
			}
		}	
	}

	/* Generates a random expression tree from nodes in stack */
	Node* treeGen(Node **unused_opnd_stack, int *u_head, int size) {	
		if (size >= 2 && rand() % 2 == 0) {
			Node *left = treeGen(unused_opnd_stack, u_head, size / 2);
			Node *right = treeGen(unused_opnd_stack, u_head, size / 2 + size % 2);
			return newOperatorNode(rand() % (N_OPTRS - 1) + 1, left, right);
		}
		else if (size >= 2) {
			Node *left = treeGen(unused_opnd_stack, u_head, 1);
			Node *right = treeGen(unused_opnd_stack, u_head, size - 1);
			return newOperatorNode(rand() % (N_OPTRS - 1) + 1, left, right);	
		}
		else {
			(*u_head)--;
			Node *opnd_node = unused_opnd_stack[*u_head];
			unused_opnd_stack[*u_head] = NULL;
			return opnd_node;	
		}
	}

	/* Count total and unused operands in expression */
	int n_total_opnds = 0, n_unused_opnds = 0;;
	for (int i = 0; i < N_OPNDS; i++) {
		if (state->operand_uses[i] == 0)
			n_unused_opnds++;	
		else 
			n_total_opnds += state->operand_uses[i];
	}

	/* Return if there are no unused operands in the expression */
	if (n_unused_opnds == 0)
		return;

	/* Initialise and build overused operand stack */
	int o_head = 0;
	Node *overused_opnd_stack[n_total_opnds + 1];
	for (int i = 0; i < n_total_opnds + 1; i++)
		overused_opnd_stack[i] = NULL;	
	stackOverusedOpnds(root, state, overused_opnd_stack, &o_head);
	
	/* Initialise unused operand stack */
	int u_head = 0;
	Node *unused_opnd_stack[n_total_opnds + 1];
	for (int i = 0; i < n_total_opnds + 1; i++)
		unused_opnd_stack[i] = NULL;	

	int n_singles = 2 * o_head - n_unused_opnds;
	int n_pairs = n_unused_opnds - n_singles;

	/* Start by replacing overused operands with single unused ones */
	for (int i = 0; i < N_OPNDS && o_head != 0 && n_singles > 0; i++) {	
		if (state->operand_uses[i] == 0) {
			/* Pop overused operand node from stack and set its
			 * respective operand to the unused operand */	
			o_head--;
			overused_opnd_stack[o_head]->operand_name = (OperandName)i;
			overused_opnd_stack[o_head] = NULL;
			
			/* Increment used/unused operand quantities respectively */	
			state->operand_uses[i]++;
			n_singles--;
			n_unused_opnds--;	
		}	
	}	

	/* Replace any remaining overused operands with pairs of unused operands */
	bool latest_is_valid = false;
	for (int i = 0, latest_i = 0; i < N_OPNDS && o_head != 0 && n_pairs > 0; i++) {
		if (!latest_is_valid && state->operand_uses[i] == 0) {
			latest_i = i;
			latest_is_valid = true;
		}
		else if (latest_is_valid && state->operand_uses[i] == 0) {
			/* Pop overused operand node from stack and replace it 
			 * in the tree with a new operator node whose two children
			 * are operand nodes with the respective two operands to be
			 * inserted */	
			o_head--;
			overused_opnd_stack[o_head]->type = OPERATOR;
			overused_opnd_stack[o_head]->operator_name = rand() % (N_OPTRS - 1) + 1;
			overused_opnd_stack[o_head]->left = newOperandNode((OperandName)latest_i);
			overused_opnd_stack[o_head]->right = newOperandNode((OperandName)i);
			
			/* Increment used/unused operand quantities respectively */
			state->operand_uses[i]++;
			state->operand_uses[latest_i]++;
			n_pairs--;
			n_unused_opnds -= 2;
			n_total_opnds++;
	
			latest_is_valid = false;
		}	
	}

	if (n_unused_opnds == 0)
		return;

	/* Choose a random operand node in the tree and "remove" it */
	Node *subtree_ins_node = root;
	Node *subtree_ins_node_parent = NULL;
	bool is_left_child = false;
	while (subtree_ins_node->type != OPERAND) {	
		subtree_ins_node_parent = subtree_ins_node; 
		if (rand() % 2 == 0) {
			subtree_ins_node = subtree_ins_node->left;
			is_left_child = true;
		}
		else {
			subtree_ins_node = subtree_ins_node->right;
			is_left_child = false;
		}
	}
	state->operand_uses[subtree_ins_node->operator_name]--;
	n_unused_opnds++;
	n_total_opnds--;
	n_pairs = n_unused_opnds / 2;
	n_singles = n_unused_opnds % 2;
	
	/* Build stack of unused operand nodes */ 	
	stackUnusedOpnds(state, unused_opnd_stack, &u_head);	

	/* Generate a random tree from unused operands by emptying the stack */	
	free(subtree_ins_node);
	subtree_ins_node = treeGen(unused_opnd_stack, &u_head, u_head);
	if (is_left_child) 
		subtree_ins_node_parent->left = subtree_ins_node;
	else 
		subtree_ins_node_parent->right = subtree_ins_node;
}

void randomBranchSwap(Node *tree, unsigned int splitChance, unsigned int selectChance, unsigned int swap_attempts) {
	/* Randomly selects a node from the tree along which splitting will occur */
	Node* getRandomSplitPoint(Node *root, int split_chance) {
		/* Return NULL (no split point) upon "bottoming out" tree */	
		if (root->type == OPERAND || 
		    root->left->type == OPERAND && root->right->type == OPERAND)
			return NULL;
	
		/* Decide whether to go deeper into the tree or not. If not then return
		 * current node */	
		if (rand() % 100 >= splitChance) 
			return root;
		
		/* Preferentially select operator node for traversal from children */
		if (root->left->type == OPERAND && root->right->type == OPERATOR) {
			if (root->right->left->type == OPERAND && root->right->right->type == OPERAND)
				return root;	
			else	
				return getRandomSplitPoint(root->right, split_chance);
		}
		if (root->right->type == OPERAND && root->left->type == OPERATOR) {
			if (root->left->left->type == OPERAND && root->left->right->type == OPERAND)
				return root;
			else 
				return getRandomSplitPoint(root->left, split_chance);
		}
		
		/* Since both children are operator nodes, randomly select which to traverse */	
		if (rand() % 2 == 0) {
			if (root->left->left->type == OPERAND && root->left->right->type == OPERAND)
				return root;
			else	
				return getRandomSplitPoint(root->left, split_chance);
		}
		else {
			if (root->right->left->type == OPERAND && root->right->right->type == OPERAND)
				return root;	
			else
				return getRandomSplitPoint(root->right, split_chance);
		}
	}

	/* On input of parent and its given child, selects random node in child's subtree and its
	 * parent. May select the child itself. */
	void selectRandomNode(int selectChance, Node *child, Node *parent, Node **selected, Node **sel_parent) {	
		while (child->type != OPERAND && rand() % 100 <= selectChance) {	
			parent = child;
			child = rand() % 2 == 0 ? child->left : child->right;
		}
		*selected = child;
		*sel_parent = parent;
	}

	Node *split_point = NULL, *sub_tree_1 = NULL, *sub_tree_2 = NULL, 
	     *st1_parent = NULL, *st2_parent = NULL;	
	do {
		split_point = getRandomSplitPoint(tree, splitChance);
		if (split_point == NULL) {	
			swap_attempts--;
			continue;
		}
		selectRandomNode(selectChance, split_point->left, split_point, &sub_tree_1, &st1_parent);
		selectRandomNode(selectChance, split_point->right, split_point, &sub_tree_2, &st2_parent);
		swap_attempts--;
	} while (swap_attempts != 0 && st1_parent == st2_parent);
	if (st1_parent != st2_parent) {
		if (sub_tree_1 == st1_parent->left && sub_tree_2 == st2_parent->left) {
			st1_parent->left = sub_tree_2;
			st2_parent->left = sub_tree_1;	
		}
		if (sub_tree_1 == st1_parent->left && sub_tree_2 == st2_parent->right) {
			st1_parent->left = sub_tree_2;
			st2_parent->right = sub_tree_1;	
		}			
		if (sub_tree_1 == st1_parent->right && sub_tree_2 == st2_parent->left) {
			st1_parent->right = sub_tree_2;
			st2_parent->left = sub_tree_1;	
		}
		if (sub_tree_1 == st1_parent->right && sub_tree_2 == st2_parent->right) {
			st1_parent->right = sub_tree_2;
			st2_parent->right = sub_tree_1;
		}
	}
}

void randomReplaceOptrs(Node *root) {
	if (root == NULL || root->type == OPERAND)
		return;
	/* Type cast operator not implemented. If implemented replace line with: 
	 * root->operator_name = rand() % N_OPTRS; */
	root->operator_name = rand() % (N_OPTRS - 1) + 1;	
	randomReplaceOptrs(root->left);	
	randomReplaceOptrs(root->right);
};

/* Returns a string representation of arithmetic expression tree */
char *exprTreeToString(Node* root, int parent_precedence, int *out_str_len) {	
	char *out_str, *left, *right, *operator;
	int precedence, left_len, right_len, operator_char_count;
	bool parenthesize;
	
	/* Unexpected case: return empty string */
	if (root == NULL) 
		return "";	

	/* Return operand name as string if node is operand */	
	if (root->type == OPERAND) {
		*out_str_len = 2;
		out_str = (char*)malloc(sizeof(char) * (*out_str_len + 2));
		strcpy(out_str, operandNameToStr(root->operand_name));
		return out_str;
	}
	
	/* Parenthesize expression if parent operator binds tighter */
	precedence = getPrecedence(root->operator_name);
	parenthesize = precedence > parent_precedence;	
	
	/* Get left subtree and right subtree expressions */
	left = exprTreeToString(root->left, precedence, &left_len);
	right = exprTreeToString(root->right, precedence, &right_len);

	/* Get this node's operator as string */
	operator = operatorNameToStr(root->operator_name);
	operator_char_count = getOperatorCharCount(root->operator_name); 

	/* Concatenate substrings to get complete sub-expression for this node */
       	if (parenthesize) {	
		*out_str_len = operator_char_count + left_len + right_len + 2;
		out_str = (char*)malloc(sizeof(char) * (*out_str_len + 1));
		strcpy(out_str, "(");	
		strcat(out_str, left);
		strcat(out_str, operator);
		strcat(out_str, right);
		strcat(out_str, ")");
	}
	else {
		*out_str_len = operator_char_count + left_len + right_len;
		out_str = (char*)malloc(sizeof(char) * (*out_str_len + 1));
		strcpy(out_str, left);
		strcat(out_str, operator);
		strcat(out_str, right);
	}
	
	/* Free left and right subexpression strings and return complete sub-expression */
	free(left);
	free(right);
	return out_str;
}

OperandName strToOperandName(char *str_begin, char *str_end) {	
	char *str_it = str_begin;

	if (str_end - str_begin <= 0)
		return INVALID_OPND;
	
	/* Skip over an 'x' (next expected character) */
	if (*str_begin != 'x')
		return INVALID_OPND;
	
	str_it++;	
	/* Get the index of the variable  and return the respective operand name */ 
	/* (i.e. index 2 for 'x2' and return OPND_X2) */
	int variable_index = (int)strtol(str_it, NULL, 10); /* TODO: replace this function with custom code if needed */
	if (variable_index >= OPND_X0 && variable_index < N_OPNDS) {
		return variable_index;
	}
	
	printf("Could not parse variable index from string: \"%s\"", str_begin);
	exit(1);
}

OperatorName strToOperatorName(char *str_begin, char *str_end) {
	/* Check whether substring has length at least 1  */	
	if (str_end - str_begin <= 0)
		return INVALID_OPTR;
	
	/* Jumps can be replaced with nested ifs */	
	if ((int)(str_end - str_begin) < 2)
		goto single_char_operators_start;
	else if ((int)(str_end - str_begin) <= 2)
		goto two_char_operators_start;

	/* Check for type cast */
	if (strncmp(str_begin, "(uint32_t)", 10) == 0 || 
	    strncmp(str_begin, "(uint64_t)", 10) == 0 || 
	    strncmp(str_begin, "(int32_t)", 9) == 0 || 
	    strncmp(str_begin, "(int64_t)", 9) == 0)
		return CAST;

	two_char_operators_start:
	/* Check for 2-character operators. */
	if (str_begin[0] == '<' && str_begin[1] == '<')
		return LSH;
	else if (str_begin[0] == '>' && str_begin[1] == '>')
        	return RSH;
    	else if (str_begin[0] == '<' && str_begin[1] == '=') 
		return SMLEQ;
	else if (str_begin[0] == '>' && str_begin[1] == '=')
        	return GRTEQ;
    	else if (str_begin[0] == '=' && str_begin[1] == '=')
        	return EQ;
    	else if (str_begin[0] == '!' && str_begin[1] == '=')
        	return NEQ;
    	else if (str_begin[0] == '&' && str_begin[1] == '&')
        	return ANDL;
    	else if (str_begin[0] == '|' && str_begin[1] == '|')
        	return ORL;

	single_char_operators_start:
	/* Check for single character operators*/	
	if (str_begin[0] == '*')
		return MUL;
	else if (str_begin[0] == '/') 
		return DIV;
	else if (str_begin[0] == '%')
		return REM;
    	else if (str_begin[0] == '+')
		return ADD;
	else if (str_begin[0] == '-')
		return SUB;
    	else if (str_begin[0] == '<')
		return SML;
	else if (str_begin[0] == '>')
		return GRT;
    	else if (str_begin[0] == '&')
		return AND;
    	else if (str_begin[0] == '^')
		return XOR;
    	else if (str_begin[0] == '|')
		return OR;
	else
        	return INVALID_OPTR;
}

int getOperatorCharCount(OperatorName operator_name) {
	switch (operator_name) {
		default:
			/* Operator not matched */	
			return -1;
			break;
		case CAST:
			/* Unique case requiring more info */
			return -2;
			break;	
		/* 1 Character operators */	
		case MUL:
		case DIV:
		case REM:	
		case ADD:
		case SUB:			
		case SML: 
		case GRT: 
		case AND:
		case XOR:
		case OR:	
			return 1;		
			break;	
		/* 2 Character operators */
		case SMLEQ:
		case GRTEQ:
		case LSH:
		case RSH:	
		case EQ:
		case NEQ:
		case ANDL:
		case ORL:
			return 2;
			break;
	}
}

int getPrecedence(OperatorName operator_name) {
	switch (operator_name) {
		case CAST:
			return 2;
			break;
		case MUL:
		case DIV:
		case REM:
			return 3;
			break;
		case ADD:
		case SUB:
			return 4;
			break;
		case LSH:
		case RSH:
			return 5;
			break;
		case SML: 
		case SMLEQ: 
		case GRT: 
		case GRTEQ:
			return 6;
			break;
		case EQ:
		case NEQ:
			return 7;
			break;
		case AND:
			return 8;
			break;
		case XOR:
			return 9;
			break;
		case OR:
			return 10;
			break;
		case ANDL:
			return 11;
			break;
		case ORL:
			return 12;
			break;
		default: /* Precedence higher than possible (error case) */
			return 16;  
	}			
}

bool isLRAssociative(int precedence) {
	switch (precedence) {
		case 2:
			return false;
			break;
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
			return true;
			break;	
	}
}

char *operatorNameToStr(OperatorName operator_name) {
	switch (operator_name) {
		default:
			return "DEFAULT_OPTR";
			break;
		case INVALID_OPTR:
			return "[]";
			break;
		case CAST:
			return "(cast)";
			break;	
		case MUL:
			return "*";
			break;
		case DIV:
			return "/";
			break;
		case REM:
			return "%";
			break;
		case ADD:
			return "+";
			break;
		case SUB:
			return "-";
			break;	
		case SML: 
			return "<";
			break;
		case GRT:
			return ">";
			break; 
		case AND:
			return "&";
			break;
		case XOR:
			return "^";
			break;
		case OR:
			return "|";
			break;
		case SMLEQ:
			return "<=";
			break;
		case GRTEQ:
			return ">=";
			break;
		case LSH:
			return "<<";
			break;
		case RSH:
			return ">>";
			break;	
		case EQ:
			return "==";
			break;
		case NEQ:
			return "!=";
			break;
		case ANDL:
			return "&&";
			break;
		case ORL:
			return "||";
			break;	
	}
}

char *operandNameToStr(OperandName operand_name) {
	switch(operand_name) {
		default:
			return "DEFAULT_OPND";
		case OPND_X0:
			return "x0";
			break;
		case OPND_X1:
			return "x1";
			break;
		case OPND_X2:
			return "x2";
			break;
		case OPND_X3:
			return "x3";
			break;
		case OPND_X4:
			return "x4";
			break;
		case OPND_X5:
			return "x5";
			break;
		case OPND_X6:
			return "x6";
			break;
		case OPND_X7:
			return "x7";
			break;
		case INVALID_OPND:	
			return "[]";
			break;
	}
}

/* Returns pointer to integer of specified type, whose value is randomised. Memory 
 * referenced by pointer must be freed manually. */
void *randomInteger(OperandDataType type) {
	void *val = malloc(8);
	
	switch (type) {
		case I64:
			*(int64_t*)val = ((int64_t)rand() << 32) | rand();
			break;
		case UI64:
			*(uint64_t*)val = ((uint64_t)rand() << 32) | rand();
			break;
		case I32:
			*(int32_t*)val = rand();
			break;
		case UI32:
			*(uint32_t*)val = rand();
			break;
		case I16:
			*(int16_t*)val = rand() & 0xFFFF;
			break;
		case UI16:
			*(uint16_t*)val = rand() & 0xFFFF;
			break;
		case I8:
			*(int8_t*)val = rand() & 0xFF;
			break;
		case UI8:
			*(uint8_t*)val = rand() & 0xFF;
			break;
	}
	
	return val;
}

char *intToStr(void *value, OperandDataType type) {
	char *str;
	switch (type) {
		/* Unsigned */
		case UI8:
			str = malloc(4);
			snprintf(str, 4, "%u", *(uint8_t*)value);
			break;
		case UI16:
			str = malloc(6);
			snprintf(str, 6, "%u", *(uint16_t*)value);
			break;
		case UI32:
			str = malloc(11);
			snprintf(str, 11, "%u", *(uint32_t*)value);
			break;	
		case UI64:
			str = malloc(21);
			snprintf(str, 21, "%lu", *(uint64_t*)value);
			break;
		/* Signed */
		case I8:
			str = malloc(5);
			snprintf(str, 5, "%d", *(int8_t*)value);
			break;
		case I16:
			str = malloc(7);
			snprintf(str, 7, "%d", *(int16_t*)value);
			break;
		case I32:
			str = malloc(12);
			snprintf(str, 12, "%d", *(int32_t*)value);
			break;
		case I64:
			str = malloc(21);
			snprintf(str, 21, "%ld", *(int64_t*)value);
			break;
	}
	return str;
}

char *intTypeToStr(OperandDataType type) {
	switch (type) {
		/* Unsigned */
		case UI8:
			return "uint8_t";
		case UI16:
			return "uint16_t";
		case UI32:
			return "uint32_t";	
		case UI64:
			return "uint64_t";
		/* Signed */
		case I8:
			return "int8_t";
		case I16:
			return "int16_t";
		case I32:
			return "int32_t";
		case I64:
			return "int64_t";
		default:
			return "unknown_type";
	}
}
