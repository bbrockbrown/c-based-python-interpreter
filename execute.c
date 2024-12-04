/*execute.c*/

//
// << Executes the given python program (user input or directly from .py file)
//	  and stores program variables into RAM. For the purpose of this project,
//    execute() only handles assignments, function calls, and 'pass'. Although 
//    functionality is somehwat limited, this form of execute() is still able to
//    handle binary expressions, including ones with variables stored in RAM. 
//	Can only do simple function calls e.g. print(), float(), int(), and input().			   >>
//
// << Brock Brown >>
// << Northwestern University >>
// << CS 211 >>
// << Fall Quarter 2024 >>
// 
// Starter code: Prof. Joe Hummel
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // true, false
#include <string.h>
#include <assert.h>
#include <math.h>     // pow(a, b)

#include "programgraph.h"
#include "ram.h"
#include "execute.h"


//
// Public functions:
//

//
// str_dup() 
//
// Allocates memory for a 'duplicate' string and returns it. Had to write this sicne
// the compiler was not recognizing the version from '<string.h>'
//
char* strdup(const char* word) {
	if (word == NULL) return NULL; 
	size_t len = strlen(word) + 1; 
	char* copy = malloc(len);     
	if (copy != NULL) {
		memcpy(copy, word, len);   
	}
	return copy;
}

//
// handle_conversion()
//
// helper function that performs the converion from string to int/float, denoted by int(s) or float(s)
//
bool handle_conversion(struct FUNCTION_CALL* func_call, struct RAM_VALUE* stored_value, struct RAM* memory, int line_num) {
	char* var_name = func_call->parameter->element_value;
	struct RAM_VALUE* var_val = ram_read_cell_by_name(memory, var_name);
	if (!var_val || var_val->value_type != RAM_TYPE_STR) {
		printf("**SEMANTIC ERROR: %s() requires a string variable (line %d)\n", func_call->function_name, line_num);
		return false;
	}

	char* endOfStr;
	if (strcmp(func_call->function_name, "int") == 0) {
		int convert_val = strtol(var_val->types.s, &endOfStr, 10);
		if (*endOfStr != '\0') {
			printf("**SEMANTIC ERROR: invalid string for int() (line %d)\n", line_num);
			return false;
		}
		stored_value->value_type = RAM_TYPE_INT;
		stored_value->types.i = convert_val;
	} 
	else if (strcmp(func_call->function_name, "float") == 0) {
		double convert_val = strtod(var_val->types.s, &endOfStr);
		if (*endOfStr != '\0') {
			printf("**SEMANTIC ERROR: invalid string for float() (line %d)\n", line_num);
			return false;
		}
		stored_value->value_type = RAM_TYPE_REAL;
		stored_value->types.d = convert_val;
	}

	return true;
}

//
// handle_function()
//
// helper function responsible for handling the Python 'input()', 'float()', and 'int()' functions
//
bool handle_function(struct FUNCTION_CALL* func_call, struct RAM_VALUE* stored_value, struct RAM* memory, int line_num) {
	if (strcmp(func_call->function_name, "input") == 0) {
		if (func_call->parameter->element_type != ELEMENT_STR_LITERAL) {
			printf("**SEMANTIC ERROR: input() must be passed a string\n");
			return false;
		}

		printf("%s", func_call->parameter->element_value);
		char line[256];
		if (fgets(line, sizeof(line), stdin) == NULL) {
			printf("**ERROR: Could not read input");
			return false;
		}

		line[strcspn(line, "\n")] = '\0';

		stored_value->value_type = RAM_TYPE_STR;
		stored_value->types.s = strdup(line);
		if (stored_value->types.s == NULL) {
			printf("**ERROR: Memory alloc failed");
			return false;
		}
		return true;
	}

	if (strcmp(func_call->function_name, "int") == 0 || strcmp(func_call->function_name, "float") == 0) {
		return handle_conversion(func_call, stored_value, memory, line_num);
	}

	printf("**ERROR: Unsupported function call '%s' (line %d)\n", func_call->function_name, line_num);
	return false;
}

//
// handle_normal_expression()
//
// helper function responsible for handling normal expression/assignment
//
bool handle_normal_expression(struct ELEMENT* rhs_elt, struct RAM_VALUE* stored_value, struct RAM* memory, int line_num) {
	switch (rhs_elt->element_type) {
		// we are assigning a normal int literal
		case ELEMENT_INT_LITERAL: {
			int value = atoi(rhs_elt->element_value);
			stored_value->value_type = RAM_TYPE_INT;
			stored_value->types.i = value;
			break;
		}

		// assigning to real number
		case ELEMENT_REAL_LITERAL: {
			double value = atof(rhs_elt->element_value);
			stored_value->value_type = RAM_TYPE_REAL;
			stored_value->types.d = value;
			break;
		}

		// assigning to string literal
		case ELEMENT_STR_LITERAL: {
			char* value = strdup(rhs_elt->element_value);
			if (value == NULL) {
				printf("**ERROR: Memory allocation failed\n");
				return false;
			}
			stored_value->value_type = RAM_TYPE_STR;
			stored_value->types.s = value;
			break;
		}

		// assigning to boolean True
		case ELEMENT_TRUE: {
			stored_value->value_type = RAM_TYPE_BOOLEAN;
			stored_value->types.i = 1;
			break;
		}

		// assigning to boolean True
		case ELEMENT_FALSE: {
			stored_value->value_type = RAM_TYPE_BOOLEAN;
			stored_value->types.i = 0;
			break;
		}

		case ELEMENT_IDENTIFIER: {
			char* rhs_name = rhs_elt->element_value;
			struct RAM_VALUE* rhs_value = ram_read_cell_by_name(memory, rhs_name);
			// sementic error, var not found (reuse error message)
			if (rhs_value == NULL) {
				printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", rhs_name, line_num);
				return false;
			}
			
			// store copy of rhs 
			*stored_value = *rhs_value;
			break;
		}

		default:
			printf("**ERROR: Unsupported RHS");
			return false;
	}
	return true;
}

//
// string_concat()
//
// helper function to handle string concatenation
//
bool string_concat(struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line_num) {
	// we are doing string concat
	size_t len = strlen(lhs.types.s) + strlen(rhs.types.s) + 1;		// account for 0 at end
	result->types.s = malloc(len * sizeof(char));
	if (result->types.s == NULL) {
		printf("**ERROR: Memory allocation failed\n");
		return false;
	}

	// concat
	result->value_type = RAM_TYPE_STR;
	snprintf(result->types.s, len, "%s%s", lhs.types.s, rhs.types.s);
	return true;
}

//
// string_comparison()
//
// helper function to handle string comparison
//
bool string_comparison(int operator, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line_num) {
	int comparison = strcmp(lhs.types.s, rhs.types.s);
	// see which operator
	switch (operator) {
		case OPERATOR_EQUAL:
			result->types.i = (comparison == 0);
			break;
		case OPERATOR_NOT_EQUAL:
			result->types.i = (comparison != 0);
			break;
		case OPERATOR_LT:
			result->types.i = (comparison < 0);
			break;
		case OPERATOR_LTE:
			result->types.i = (comparison <= 0);
			break;
		case OPERATOR_GT:
			result->types.i = (comparison > 0);
			break;
		case OPERATOR_GTE:
			result->types.i = (comparison >= 0);
			break;
		default:
			printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line_num);
			return false;
	}
	result->value_type = RAM_TYPE_BOOLEAN;
	return true;
}

//
// number_comparison()
//
// helper function to handle comparison between numbers
//
bool number_comparison(int operator, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line_num) {
	result->value_type = RAM_TYPE_BOOLEAN;

	// two integers
	if (lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_INT) {
		switch (operator) {
			case OPERATOR_EQUAL:    result->types.i = (lhs.types.i == rhs.types.i); break;
			case OPERATOR_NOT_EQUAL: result->types.i = (lhs.types.i != rhs.types.i); break;
			case OPERATOR_LT:       result->types.i = (lhs.types.i < rhs.types.i); break;
			case OPERATOR_LTE:      result->types.i = (lhs.types.i <= rhs.types.i); break;
			case OPERATOR_GT:       result->types.i = (lhs.types.i > rhs.types.i); break;
			case OPERATOR_GTE:      result->types.i = (lhs.types.i >= rhs.types.i); break;
		}
		return true;
	}

	// two reals
	else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_REAL) {
		switch (operator) {
			case OPERATOR_EQUAL:    result->types.i = (lhs.types.d == rhs.types.d); break;
			case OPERATOR_NOT_EQUAL: result->types.i = (lhs.types.d != rhs.types.d); break;
			case OPERATOR_LT:       result->types.i = (lhs.types.d < rhs.types.d); break;
			case OPERATOR_LTE:      result->types.i = (lhs.types.d <= rhs.types.d); break;
			case OPERATOR_GT:       result->types.i = (lhs.types.d > rhs.types.d); break;
			case OPERATOR_GTE:      result->types.i = (lhs.types.d >= rhs.types.d); break;
		}
		return true;
	}

	// nothing else
	else {
		printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line_num);
		return false;
	}

	return true;
}

//
// change_numeric_types()
//
// given two pointers to RAM_VALUE structs representing the lhs & rhs, changes the value_type of these structs depending
// on the combination of values (int & real --> both real)
//
void change_numeric_types(struct RAM_VALUE* lhs, struct RAM_VALUE* rhs) {
	// if we are doing something with real and int --> convert int to real
	if (lhs->value_type == RAM_TYPE_INT && rhs->value_type == RAM_TYPE_REAL) {
		lhs->types.d = (double)lhs->types.i;
		lhs->value_type = RAM_TYPE_REAL;
	} 
	else if (lhs->value_type == RAM_TYPE_REAL && rhs->value_type == RAM_TYPE_INT) {
		rhs->types.d = (double)rhs->types.i;
		rhs->value_type = RAM_TYPE_REAL;
	}
}

// 
// handle_integer_ops()
//
// given an operator and lhs & rhs, performs any supported operations between two integers and modifies the result via 
// a pointer, returning T/F depending on successfulness
bool handle_integer_ops(int operator, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line_num) {
	result->value_type = RAM_TYPE_INT;
	switch (operator) {
		// we are doing (+) or (-) or (*)
		case OPERATOR_PLUS:    result->types.i = lhs.types.i + rhs.types.i; break;
		case OPERATOR_MINUS:   result->types.i = lhs.types.i - rhs.types.i; break;
		case OPERATOR_ASTERISK: result->types.i = lhs.types.i * rhs.types.i; break;

		// we are doing division
		case OPERATOR_DIV:
			if (rhs.types.i == 0) {
				printf("**ZeroDivisionError: division by zero (line %d)\n", line_num);
				return false;
			}
			result->types.i = lhs.types.i / rhs.types.i;
			break;

		// we are doing modulo
		case OPERATOR_MOD:
			result->types.i = lhs.types.i % rhs.types.i;
			break;

		// we are doing pow(a, b)
		case OPERATOR_POWER:
			result->types.i = (int)pow(lhs.types.i, rhs.types.i);
			break;

		// unknown operator
		default:
			printf("**ERROR: Unsupported operator (line %d)\n", line_num);
			return false;
	}
	return true;
}

// 
// handle_real_ops()
//
// given an operator and lhs & rhs, performs any supported operations between two real numbers and modifies the result via 
// a pointer, returning T/F depending on successfulness
bool handle_real_ops(int operator, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line_num) {
	result->value_type = RAM_TYPE_REAL;
	switch (operator) {
		// we are doing (+) or (-) or (*)
		case OPERATOR_PLUS:    result->types.d = lhs.types.d + rhs.types.d; break;
		case OPERATOR_MINUS:   result->types.d = lhs.types.d - rhs.types.d; break;
		case OPERATOR_ASTERISK: result->types.d = lhs.types.d * rhs.types.d; break;

		// we are doing division
		case OPERATOR_DIV: {
			if (rhs.types.d == 0.0) {
				printf("**ZeroDivisionError: division by zero (line %d)\n", line_num);
				return false;
			}
			result->types.d = lhs.types.d / rhs.types.d;
			break;
		}

		// we are doing modulo
		case OPERATOR_MOD:
			result->types.d = fmod(lhs.types.d, rhs.types.d);
			break;

		// we are doing pow(a, b)
		case OPERATOR_POWER: {
			result->types.d = pow(lhs.types.d, rhs.types.d);
			break;
		}

		// unknown operator
		default: {
			printf("**ERROR: Unsupported operator (line %d)\n", line_num);
			return false;
		}
	}
	return true;
}

// 
// is_relational_op()
//
// given an operator, returns T/F depending on if the operator is a relational operator
//
bool is_relational_op(int operator) {
	return operator == OPERATOR_EQUAL ||
		operator == OPERATOR_NOT_EQUAL ||
		operator == OPERATOR_LT ||
		operator == OPERATOR_LTE ||
		operator == OPERATOR_GT ||
		operator == OPERATOR_GTE;
}

//
// deref_pointer()
//
// given a pointer to a RAM_VALUE struct, dereferences the pointer and returns a bool depending on if successful or not
//
bool deref_pointer(struct RAM_VALUE* value, struct RAM* memory, int line_num) {
	if (value->value_type != RAM_TYPE_PTR) return true;
	int addr = value->types.i;
	if (addr < 0 || addr >= memory->capacity) {
		printf("**SEMANTIC ERROR: lhs pointer contains invalid address (line %d)\n", line_num);
		return false;
	}
	
	struct RAM_VALUE* deref_val = ram_read_cell_by_addr(memory, addr);
	if (!deref_val) {
		printf("**SEMANTIC ERROR: lhs pointer contains invalid address (line %d)\n", line_num);
		return false;
	}

	*value = *deref_val;
	return true;
}

//
// handle_pointer_arithmetic()
//
// given two RAM_VALUE structs that represent a pointer and int, adds the int value to the pointer value to represent
// changing the address of the pointer. Modifies the result via a pointer and returns bool depending on success
//
bool handle_pointer_arithmetic(int operator, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line_num) {
	int new_addr = (lhs.value_type == RAM_TYPE_PTR) ? lhs.types.i : rhs.types.i;
	int to_add = (lhs.value_type == RAM_TYPE_INT) ? lhs.types.i : rhs.types.i;

	// we are adding
	if (operator == OPERATOR_PLUS) {
		new_addr += to_add;
	}
	// subtracting
	else if (operator == OPERATOR_MINUS) {
		new_addr -= to_add;
	}
	// unsupported operator
	else {
		printf("**SEMANTIC ERROR: invalid operand types for pointer arithmetic (line %d)\n", line_num);
		return false;
	}

	result->value_type = RAM_TYPE_PTR;
	result->types.i = new_addr;
	return true;
}

//
// determine_op_result
//
// Given an operator, lhs & rhs, line number, and a pointer to a result of RAM_VALUE, this function handles
// all types of variables when doing +, -, *, /, **, and %. 
// 
bool determine_op_result(int operator, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line_num, struct RAM* memory, bool lhs_deref, bool rhs_deref) {
	// see what types are in the operation

	// check if we are dereferencing any pointers if needed
	// if (lhs_deref && !deref_pointer(&lhs, memory, line_num)) return false;
	// if (rhs_deref && !deref_pointer(&rhs, memory, line_num)) return false;

	// handle any string operations
	if (lhs.value_type == RAM_TYPE_STR || rhs.value_type == RAM_TYPE_STR) {
		// both operands are strings and operation is (+) --> string concat
		if (operator == OPERATOR_PLUS && lhs.value_type == RAM_TYPE_STR && rhs.value_type == RAM_TYPE_STR) {
			return string_concat(lhs, rhs, result, line_num);
		}

		// check if we are doing relational stuff with strings
		else if (lhs.value_type == RAM_TYPE_STR && rhs.value_type == RAM_TYPE_STR) {
			if (operator == OPERATOR_EQUAL || operator == OPERATOR_NOT_EQUAL || operator == OPERATOR_LT ||
			operator == OPERATOR_LTE || operator == OPERATOR_GT || operator == OPERATOR_GTE) {
				return string_comparison(operator, lhs, rhs, result, line_num);
			}
		}
		printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line_num);
		return false;
	}

	// handle pointer arithmetic if we are adding numbers to a pointer (change address)
	if ((lhs.value_type == RAM_TYPE_PTR && rhs.value_type == RAM_TYPE_INT && !lhs_deref) || 
	(rhs.value_type == RAM_TYPE_PTR && lhs.value_type == RAM_TYPE_INT && !rhs_deref)) {
		return handle_pointer_arithmetic(operator, lhs, rhs, result, line_num);
	}

	// check if we need to change from int --> real and perform operation
	change_numeric_types(&lhs, &rhs);

	// check if we are dealing with relational operators
	if (is_relational_op(operator)) {
		return number_comparison(operator, lhs, rhs, result, line_num);
	}

	if (lhs.value_type == RAM_TYPE_INT && rhs.value_type == RAM_TYPE_INT) {
		return handle_integer_ops(operator, lhs, rhs, result, line_num);
	}
	else if (lhs.value_type == RAM_TYPE_REAL && rhs.value_type == RAM_TYPE_REAL) {
		return handle_real_ops(operator, lhs, rhs, result, line_num);
	}

	// otherwise unsupported 
	printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line_num);
	return false;
}


//
// retrieve_value
//
// Given a pointer to the LHS or RHS of the assignment, returns the appropriate
// integer value stored in memory
//
bool retrieve_value(struct UNARY_EXPR* op, struct RAM* memory, struct RAM_VALUE* value, bool* success, int line_num) {
	*success = false;
      struct ELEMENT* element = op->element;

	// check if operator is '&'
	if (op->expr_type == UNARY_ADDRESS_OF) {
		if (element->element_type != ELEMENT_IDENTIFIER) {
			printf("**SEMANTIC ERROR: '&' can only be used with an identifier (line %d)\n", line_num);
			return false;
		}

		char* name = element->element_value;
		int addr = ram_get_addr(memory, name);
		// make sure memory address is valid
		if (addr == -1) {
			printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", name, line_num);
			return false;
		}

		value->value_type = RAM_TYPE_PTR;
		value->types.i = addr;
		*success = true;
		return true;
	}

	// check if operatort is '*'
	if (op->expr_type == UNARY_PTR_DEREF) {
		char* name = element->element_value;
		struct RAM_VALUE* ptr_val = ram_read_cell_by_name(memory, name);
		// make sure the ptr is valid
		if (!ptr_val) {
			printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", name, line_num);
			return false;
		}
		// make sure var is actually a ptr
		if (ptr_val->value_type != RAM_TYPE_PTR) {
			printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line_num);
			return false;
		}
		// make sure addr of ptr is within memory range
		int addr = ptr_val->types.i;
		if (addr < 0 || addr >= memory->capacity) {
			printf("**SEMANTIC ERROR: '%s' contains invalid address (line %d)\n", name, line_num);
			return false;
		}
		// make sure deref val is valid
		struct RAM_VALUE* deref_val = ram_read_cell_by_addr(memory, addr);
		if (!deref_val) {
			printf("**SEMANTIC ERROR: '%s' contains invalid address (line %d)\n", name, line_num);
			return false;
		}

		*value = *deref_val;
		*success = true;
		return true;
	}
	
	switch (element->element_type) {
		// elt is an int literal
		case ELEMENT_INT_LITERAL: {
			value->value_type = RAM_TYPE_INT;
			value->types.i = atoi(element->element_value);
			*success = true;
			return true;
		}

		// elt is a real literal
		case ELEMENT_REAL_LITERAL: {
			value->value_type = RAM_TYPE_REAL;
			value->types.d = atof(element->element_value);
			*success = true;
			return true;
		}

		// elt is a string literal
		case ELEMENT_STR_LITERAL: {
			value->value_type = RAM_TYPE_STR;
			value->types.s = strdup(element->element_value);
			if (value->types.s == NULL) {
			printf("**ERROR: Memory allocation failed\n");
			return false;
			}
			*success = true;
			return true;
		}

		// elt is true bool
		case ELEMENT_TRUE: {
			value->value_type = RAM_TYPE_BOOLEAN;
			value->types.i = 1;  
			*success = true;
			return true;
		}

		// elt is false bool
		case ELEMENT_FALSE: {
			value->value_type = RAM_TYPE_BOOLEAN;
			value->types.i = 0;
			*success = true;
			return true;
		}

		// elt is a variable
		case ELEMENT_IDENTIFIER: {
			// get var name
			char* name = element->element_value;
			struct RAM_VALUE* ram_value = ram_read_cell_by_name(memory, name);
			// identifier var not found
			if (ram_value == NULL) {
				printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", name, line_num);
				return 0;
			}

			*value = *ram_value;
			*success = true;
			return true;
		}

		// unknown operand
		default:
			printf("**ERROR: Unsupported operand type (line %d)\n", line_num); 
			return false;
	}
}

//
// execute_binary_expression
//
// Given an expression denoting the RHS of a statement, performs the given
// binary operation and returns the result. Supports the following operators:
// +, -, *, /, %, **, with either integer literals or identifiers being the
// ones involved in said operations.
//
bool execute_binary_expression(struct EXPR* expr, struct RAM_VALUE* result, struct RAM* memory, int line_num) {
	bool success = false;
	
	// get lhs val
	struct UNARY_EXPR* lhs = expr->lhs;
	
	struct RAM_VALUE lhs_val = { .value_type = RAM_TYPE_NONE};

	// check if we got the value correctly, error printed out by retrieve_value()
	if (!retrieve_value(lhs, memory, &lhs_val, &success, line_num) || !success) {
		return false;
	}
	
	// make sure it is a binary expression
	// if not, copy lhs directly
	if (!expr->isBinaryExpr) {
		*result = lhs_val;
		return true;
	}

	// get rhs val and check validity
	struct UNARY_EXPR* rhs = expr->rhs;
	struct RAM_VALUE rhs_val = { .value_type = RAM_TYPE_NONE};

	// check if we got the value correctly, error printed out by retrieve_value()
	if (!retrieve_value(rhs, memory, &rhs_val, &success, line_num) || !success) {
		return false;
	}

	return determine_op_result(expr->operator, lhs_val, rhs_val, result, line_num, memory, lhs->expr_type == UNARY_PTR_DEREF, rhs->expr_type == UNARY_PTR_DEREF);
}


// 
// handle_pointer_assignment()
//
// given the rhs of an assignment of a pointer, handles dereferencing the pointer and performing operations with it, returns
// T/F depending on success
//
bool handle_pointer_assignment(struct STMT_ASSIGNMENT* assignment, struct VALUE* rhs, struct RAM* memory, int line_num) {
	char* ptr_name = assignment->var_name;

	// make sure ptr exists
	struct RAM_VALUE* ptr_val = ram_read_cell_by_name(memory, ptr_name);
	if (!ptr_val) {
		printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", ptr_name, line_num);
		return false;
	}

	// make sure var == ptr
	if (ptr_val->value_type != RAM_TYPE_PTR) {
		printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line_num);
		return false;
	}

	// make sure addr in range
	int addr = ptr_val->types.i;
	if (addr < 0 || addr >= memory->capacity) {
		printf("**SEMANTIC ERROR: '%s' contains invalid address (line %d)\n", ptr_name, line_num);
		return false;
	}

	// prepare ram to store this
	struct RAM_VALUE stored_value = { .value_type = RAM_TYPE_NONE };

	// do rhs
	if (rhs->value_type == VALUE_FUNCTION_CALL) {
		if (!handle_function(rhs->types.function_call, &stored_value, memory, line_num)) {
			return false;
		}
	} else if (rhs->value_type == VALUE_EXPR) {
		struct EXPR* expr = rhs->types.expr;

		if (expr->isBinaryExpr) {
			if (!execute_binary_expression(expr, &stored_value, memory, line_num)) {
				return false;
			}
			} else {
			if (!handle_normal_expression(expr->lhs->element, &stored_value, memory, line_num)) {
				return false;
			}
		}
	}

	// make sure dereferencing works
	if (!ram_write_cell_by_addr(memory, stored_value, addr)) {
		printf("**ERROR: Could not write value to memory location\n");
		return false;
	}

	return true; 
}

//
// handle_unary_address_of()
//
// given a pointer to a UNARY_EXPR* struct, assigns stored_value to that pointer. returns T/F depending on success
//
bool handle_unary_address_of(struct UNARY_EXPR* expr, struct RAM_VALUE* stored_value, struct RAM* memory, int line_num) {
	bool success = false;

	if (!retrieve_value(expr, memory, stored_value, &success, line_num)) {
		return false;
	}

	if (success) {
		stored_value->value_type = RAM_TYPE_PTR;
	}
	return true;
}

//
// handle_unary_pointer_deref()
//
// given a pointer to a UNARY_EXPR* struct, dereferences the pointer and stores the result in stored_value. returns T/F depending on success
//
bool handle_unary_pointer_deref(struct UNARY_EXPR* expr, struct RAM_VALUE* stored_value, struct RAM* memory, int line_num) {
	char* ptr_name = expr->element->element_value;
	struct RAM_VALUE* ptr_val = ram_read_cell_by_name(memory, ptr_name);

	// make sure ptr is valid
	if (!ptr_val) {
		printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", ptr_name, line_num);
		return false;
	}

	// check if the variable is actually a pointer
	if (ptr_val->value_type != RAM_TYPE_PTR) {
		printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", line_num);
		return false;
	}

	int addr = ptr_val->types.i;
	if (addr < 0 || addr >= memory->capacity) {
		printf("**SEMANTIC ERROR: '%s' contains invalid address (line %d)\n", ptr_name, line_num);
		return false;
	}

	struct RAM_VALUE* deref_val = ram_read_cell_by_addr(memory, addr);
	if (!deref_val) {
		printf("**SEMANTIC ERROR: '%s' contains invalid address (line %d)\n", ptr_name, line_num);
		return false;
	}

	*stored_value = *deref_val;
	return true;
}

//
// process_rhs()
//
// given a rhs of an assignment, properly handles assignment if dealing with expression, single unary expr, etc.
// returns T/F depending on if successful
//
bool process_rhs(struct VALUE* rhs, struct RAM_VALUE* stored_value, struct RAM* memory, int line_num) {
	// check if its a function call
	if (rhs->value_type == VALUE_FUNCTION_CALL) {
		if (!handle_function(rhs->types.function_call, stored_value, memory, line_num)) {
			return false;
		}
	}
	// check if we are dealing with binary expr or normal assignment
	else if (rhs->value_type == VALUE_EXPR) {
		struct EXPR* expr = rhs->types.expr;

		if (expr->isBinaryExpr) {
			bool success = execute_binary_expression(expr, stored_value, memory, line_num);

			// make sure expr executed
			if (!success) {
				return false;
			}

			// check if lhs and rhs indicate pointer arithmetic
			if (expr->lhs->element->element_type == ELEMENT_IDENTIFIER &&
			expr->rhs->element->element_type == ELEMENT_INT_LITERAL &&
			expr->lhs->expr_type != UNARY_PTR_DEREF &&
			stored_value->value_type == RAM_TYPE_PTR) {
				stored_value->value_type = RAM_TYPE_PTR;
			}
			
			return true;
		}

		// '&' operator
		if (expr->lhs->expr_type == UNARY_ADDRESS_OF) {
			return handle_unary_address_of(expr->lhs, stored_value, memory, line_num);
		}

		// '*' operator (dereference)
		if (expr->lhs->expr_type == UNARY_PTR_DEREF) {
			return handle_unary_pointer_deref(expr->lhs, stored_value, memory, line_num);
		}

		// normal expression
		return handle_normal_expression(expr->lhs->element, stored_value, memory, line_num);
	}

	return false;
}	

//
// execute_assignment
//
// Given pointers to statement and memory, assigns a variable to its givn value.
// Returns true if the statement was executed (variable assigned) correctly, false
// otherwise. 
//
bool execute_assignment(struct STMT* stmt, struct RAM* memory) {
	// make sure stmt is actually assigning something
	assert(stmt->stmt_type == STMT_ASSIGNMENT);

	struct STMT_ASSIGNMENT* assignment = stmt->types.assignment;
	struct VALUE* rhs = assignment->rhs;

	// get var name
	char* name = assignment->var_name;

	// prepare ram to store this
	struct RAM_VALUE stored_value = { .value_type = RAM_TYPE_NONE };

	// check if we are doing pointer-based assignment (*p = ...)
	if (assignment->isPtrDeref) {
		return handle_pointer_assignment(assignment, rhs, memory, stmt->line);
	}

	// if not assignment, then must be function call or expression
	if (!process_rhs(rhs, &stored_value, memory, stmt->line)) {
		return false;
	}

	// write our value to memory
	if (!ram_write_cell_by_name(memory, stored_value, name)) {
		printf("**ERROR: Could not write variable to memory. Variable: %s\n", name);
		if (stored_value.value_type == RAM_TYPE_STR) {
			free(stored_value.types.s);
		}
		return false;
	}

	// successful assignment
	return true;
}


//
// execute_function_call
//
// Given a pointer to a statement and memory, executes the function
// call specified by the pointer. Specifically, it deals with printing
// empty print statements (new line) and printing string literals via printf()
//
bool execute_function_call(struct STMT* stmt, struct RAM* memory) {
	// make sure the stmt is actually a function call
	assert(stmt->stmt_type == STMT_FUNCTION_CALL);

	struct STMT_FUNCTION_CALL* call = stmt->types.function_call;

	// check if it's just print() or print() with string literals
	if (strcmp(call->function_name, "print") == 0) {
		struct ELEMENT* parameter = call->parameter;

		if (parameter == NULL) {
			printf("\n");
		}
		else {
			switch (parameter->element_type) {
				case ELEMENT_INT_LITERAL:
					// convert string to integer and print
					printf("%d\n", atoi(parameter->element_value));
					break;

				case ELEMENT_REAL_LITERAL:
					// convert string to double and print
					printf("%f\n", atof(parameter->element_value));
					break;

				case ELEMENT_STR_LITERAL:
					// normal string
					printf("%s\n", parameter->element_value);
					break;

				case ELEMENT_TRUE:
					// true
					printf("True\n");
					break;

				case ELEMENT_FALSE:
					// false
					printf("False\n");
					break;

				case ELEMENT_NONE:
					// None type
					printf("None\n");
					break;

				case ELEMENT_IDENTIFIER: {
					// if identifier, get the value based on identiifer
					struct RAM_VALUE* value = ram_read_cell_by_name(memory, parameter->element_value);
					if (value == NULL) {
						printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", parameter->element_value, stmt->line);
						return false;
					}

					// print the value
					switch (value->value_type) {
						case RAM_TYPE_INT:
							printf("%d\n", value->types.i);
							break;
						case RAM_TYPE_REAL:
							printf("%f\n", value->types.d);
							break;
						case RAM_TYPE_BOOLEAN:
							printf("%s\n", value->types.i ? "True" : "False");
							break;
						case RAM_TYPE_STR:
							printf("%s\n", value->types.s);
							break;
						case RAM_TYPE_PTR:
							printf("%d\n", value->types.i);
							break;
						default:
							printf("**ERROR: Unsupported variable type for '%s'\n", parameter->element_value);
						return false;
					}
					break;
				}

				default:
					// unknown argument for print
					printf("**ERROR: Unsupported argument type for print\n");
					return false;
			}
		}
		// successfully did the function call
		return true;
	}
	
	// function call was not print, input, float(s), or int(s)
	return false;
}


//
// execute
//
// Given a nuPython program graph and a memory, 
// executes the statements in the program graph.
// If a semantic error occurs (e.g. type error),
// and error message is output, execution stops,
// and the function returns.
//
void execute(struct STMT* program, struct RAM* memory)
{
	struct STMT* stmt = program;
	while (stmt != NULL) {
		// handle statements differently based on what they're doing
		switch (stmt->stmt_type) {
			case STMT_ASSIGNMENT: {
				int line = stmt->line;
				if (!execute_assignment(stmt, memory)) {
					// assignment not done properly
					return;
				}
				stmt = stmt->types.assignment->next_stmt;
				break;
			}

			case STMT_FUNCTION_CALL: {
				int line = stmt->line;
				if (!execute_function_call(stmt, memory)) {
					// function (print) call was not done properly
					return;
				}
				stmt = stmt->types.function_call->next_stmt;
				break;
			}

			case STMT_PASS: {
				int line = stmt->line;
				stmt = stmt->types.pass->next_stmt;
				break;
			}

			case STMT_WHILE_LOOP: {
				struct STMT_WHILE_LOOP* while_loop = stmt->types.while_loop;
				bool continueLoop = true;

				struct RAM_VALUE condition_result = { .value_type = RAM_TYPE_NONE };

				// evaluate the loop conditional 
				continueLoop = execute_binary_expression(while_loop->condition, &condition_result, memory, stmt->line);

				// if the condition is satisfied, go to the first statement in the loop body
				if (continueLoop) {
					if (condition_result.value_type == RAM_TYPE_BOOLEAN) {
						continueLoop = condition_result.types.i;
					} 
					else if (condition_result.value_type == RAM_TYPE_INT) {
						continueLoop = (condition_result.types.i != 0);
					} 
					else if (condition_result.value_type == RAM_TYPE_REAL) {
						continueLoop = (condition_result.types.d != 0.0);
					} 
					else {
						printf("**SEMANTIC ERROR: invalid while loop condition type (line %d)\n", stmt->line);
						stmt = while_loop->next_stmt;
						break;
					}

					if (!continueLoop) {
						stmt = while_loop->next_stmt;
					}
					else {
						stmt = while_loop->loop_body;
					}
				}

				// if not satisfied, go to statement after the loop
				else {
					if (condition_result.value_type == RAM_TYPE_NONE) {
						return;
					}
					stmt = while_loop->next_stmt;
				}
				
				break;
			}
		}
	}
}
