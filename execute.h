/*execute.h*/

//
// Executes nuPython program, given as a Program Graph.
// 
// Prof. Joe Hummel
// Northwestern University
// CS 211
//

#pragma once
#include <string.h>
#include "programgraph.h"
#include "ram.h"

//
// Public functions:
//

//
// handle_function()
//
// helper function responsible for handling the Python 'input()', 'float()', and 'int()' functions
//
bool handle_function(struct FUNCTION_CALL* func_call, struct RAM_VALUE* stored_value, struct RAM* memory, int line_num);

//
// handle_conversion()
//
// helper function that performs the converion from string to int/float, denoted by int(s) or float(s)
//
bool handle_conversion(struct FUNCTION_CALL* func_call, struct RAM_VALUE* stored_value, struct RAM* memory, int line_num);

//
// handle_normal_expression()
//
// helper function responsible for handling normal expression/assignment
//
bool handle_normal_expression(struct ELEMENT* rhs_elt, struct RAM_VALUE* stored_value, struct RAM* memory, int line_num);

//
// string_concat()
//
// helper function to handle string concatenation
//
bool string_concat(struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line_num);

//
// string_comparison()
//
// helper function to handle string comparison
//
bool string_comparison(int operator, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line_num);

//
// number_comparison()
//
// helper function to handle comparison between numbers
//
bool number_comparison(int operator, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line_num);

// determine_op_result
//
// Given an operator, lhs & rhs, line number, and a pointer to a result of RAM_VALUE, this function handles
// all types of variables when doing +, -, *, /, **, and %. 
//
bool determine_op_result(int operator, struct RAM_VALUE lhs, struct RAM_VALUE rhs, struct RAM_VALUE* result, int line_num);

//
// retrieve_value
//
// Given a pointer to the LHS or RHS of an expression, returns the appropriate
// integer value of the expression
//
bool retrieve_value(struct UNARY_EXPR* op, struct RAM* memory, struct RAM_VALUE* value, bool* success, int line_num);

//
// execute_binary_expression
//
// Given an expression denoting the RHS of a statement, performs the given
// binary operation and returns the result. Supports the following operators:
// +, -, *, /, %, **, with either integer literals or identifiers being the
// ones involved in said operations.
// NOTES:
//    - must be called within execute_assignment()
//    - must handle semantic errors, output message, and return false
//    - think about how this function needs to return true/false AND integer result of operation, only
//      if it is done successfully
//          - can use 1 of 3 ways to handle this as shown in PDF
//    - get value of LHS and RHS THEN perform operation
//    - could write 'retrieve_value()' helper function that is passed a pointer to the LHS or RHS and returns the 
//      appropriate integer value
//
//    ** NEW CONDITIONS **
//    1. if both operands are int, result is int
//    2. if both operands are real, result is real
//    3. If one operand is integer and other is real, do computation using both as reals and result is real
//    4. if both operands are string and operator is '+', perform string concat.
//    5. if dividing by 0 or 0.0, report semantic error: “ZeroDivisionError: division by zero” and stop
//    6. all other combinations are illegal and should result in semantic errors; use printf
//    7. '%' doesn't work w/ reals --> use fmod(). 
//    8. when doing string concat, need to dynamically allocate memory for result + '0' at end of str
//
bool execute_binary_expression(struct EXPR* expr, struct RAM_VALUE* result, struct RAM* memory, int line_num);

//
// execute_assignment
//
// Given pointers to statement and memory, assigns a variable to its givn value.
// Returns true if the statement was executed (variable assigned) correctly, false
// otherwise. 
//
bool execute_assignment(struct STMT* stmt, struct RAM* memory);

//
// execute_function_call
//
// Given a pointer to a statement and memory, executes the function
// call specified by the pointer. Specifically, it deals with printing
// empty print statements (new line) and printing string literals via printf().
// Returns true if function call is done successfully, false otherwise.
//
bool execute_function_call(struct STMT* stmt, struct RAM* memory);

//
// execute
//
// Given a nuPython program graph and a memory, 
// executes the statements in the program graph.
// If a semantic error occurs (e.g. type error),
// and error message is output, execution stops,
// and the function returns.
//
void execute(struct STMT* program, struct RAM* memory);

