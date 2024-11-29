/*ram.c*/

//
// << Stores variables within nuPython script using RAM via vector-like dynamic array,
// 	with each index of the array containing an identifier (variable name) and
//    value (variable value), or a 'ram cell.'  Variables can be written to memory, 
//	via address or name (identifier), but to write to an address, the address must be
// 	valid (aka a variable must be written by name first to make the address valid). After
//	being written to memory, variables can be read by name or by address, and their corresponding
//	values will be displayed. To see all variables and their values, one can use the function 
//	'ram_print().'	>>
//
// << Brock Brown >>
//
// Template: Prof. Joe Hummel
// Northwestern University
// CS 211
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // true, false
#include <string.h>
#include <assert.h>

#include "ram.h"


//
// Public functions:
//

//
// ram_init
//
// Returns a pointer to a dynamically-allocated memory
// for storing nuPython variables and their values. All
// memory cells are initialized to the value None.
//
struct RAM* ram_init(void)
{
	// allocate memory for RAM struct
	struct RAM* memory = (struct RAM*)malloc(sizeof(struct RAM));
	if (memory == NULL) {
		return NULL;
	}

	// initial RAM field values
	memory->num_values = 0;
	memory->capacity = 4;

	// allocate memory for variable cells/array
	memory->cells = (struct RAM_CELL*)malloc(memory->capacity * sizeof(struct RAM_CELL));
	if (memory->cells == NULL) {
		free(memory);
		return NULL;
	}

	// initial value for each memory cell is null and type of none
	for (int i = 0; i < memory->capacity; i++) {
		memory->cells[i].identifier = NULL;
		memory->cells[i].value.value_type = RAM_TYPE_NONE;
	}

	return memory;
}


//
// ram_destroy
//
// Frees the dynamically-allocated memory associated with
// the given memory. After the call returns, you cannot
// use the memory.
//
void ram_destroy(struct RAM* memory)
{
	if (memory == NULL) {
		return; 	// aka nothing to free
	}

	for (int i = 0; i < memory->capacity; i++) {
		// for each var in cells, free it so long as it is not NULL
		// question: do we have to set the identifier to NULL after freeing it?
		if (memory->cells[i].identifier != NULL) {
			free(memory->cells[i].identifier);
			memory->cells[i].identifier = NULL;
		}

		// need to free allocated strings since they are duplicated
		if (memory->cells[i].value.value_type == RAM_TYPE_STR && memory->cells[i].value.types.s != NULL) {
			free(memory->cells[i].value.types.s);
			memory->cells[i].value.types.s = NULL;
		}
	}

	// free the array of cells
	if (memory->cells != NULL) {
		free(memory->cells);
		memory->cells = NULL;
	}

	// free the actual RAM struct
	free(memory);
}


//
// ram_get_addr
// 
// If the given identifier (e.g. "x") has been written to 
// memory, returns the address of this value --- an integer
// in the range 0..N-1 where N is the number of values currently 
// stored in memory. Returns -1 if no such identifier exists 
// in memory. 
// 
// NOTE: a variable has to be written to memory before you can
// get its address. Once a variable is written to memory, its
// address never changes.
//
int ram_get_addr(struct RAM* memory, char* identifier)
{
	// nothing to search for
	if (memory == NULL || identifier == NULL) {
		return -1;
	}

	// we want to loop through our memory cell array and see if any of the identifiers match
	// the address we return is basically the index within the cell array, since the addr
	// must be within the range [0, N-1], where N = # of vals stored in memory
	for (int i = 0; i < memory->num_values; i++) {
		if (strcmp(memory->cells[i].identifier, identifier) == 0 && memory->cells[i].identifier != NULL) {
			return i;
		}
	}

	// identifier not found
	return -1;
}


//
// ram_read_cell_by_addr
//
// Given a memory address (an integer in the range 0..N-1), 
// returns a COPY of the value contained in that memory cell.
// Returns NULL if the address is not valid.
//
// NOTE: this function allocates memory for the value that
// is returned. The caller takes ownership of the copy and 
// must eventually free this memory via ram_free_value().
//
// NOTE: a variable has to be written to memory before its
// address becomes valid. Once a variable is written to memory,
// its address never changes.
//
struct RAM_VALUE* ram_read_cell_by_addr(struct RAM* memory, int address)
{
	// nothing to search OR address is out of range
	if (memory == NULL || address < 0 || address >= memory->num_values) {
		return NULL;
	}

	// we already have index of variable we want, so allocate space and retrieve it
	struct RAM_VALUE* copy = (struct RAM_VALUE*)malloc(sizeof(struct RAM_VALUE));
	if (copy == NULL) {
		return NULL;	// memory allocation failed?
	}

	copy->value_type = memory->cells[address].value.value_type;

	switch (copy->value_type) {
		case RAM_TYPE_INT:
		case RAM_TYPE_PTR:
		case RAM_TYPE_BOOLEAN:
			copy->types.i = memory->cells[address].value.types.i;
			break;

		case RAM_TYPE_REAL:
			copy->types.d = memory->cells[address].value.types.d;
			break;

		case RAM_TYPE_STR:
			if (memory->cells[address].value.types.s != NULL) {
				copy->types.s = strdup(memory->cells[address].value.types.s);
				if (copy->types.s == NULL) {
					free(copy);  // strdup fails
					return NULL;
				}
			} else {
				copy->types.s = NULL;
			}
			break;

		case RAM_TYPE_NONE:
			// don't need to do anything for None type
			break;

		default:
			// invalid var. type, so free it and return NULL
			free(copy);
			return NULL;
	}
	return copy;
}


// 
// ram_read_cell_by_name
//
// If the given name (e.g. "x") has been written to 
// memory, returns a COPY of the value contained in memory.
// Returns NULL if no such name exists in memory.
//
// NOTE: this function allocates memory for the value that
// is returned. The caller takes ownership of the copy and 
// must eventually free this memory via ram_free_value().
//
struct RAM_VALUE* ram_read_cell_by_name(struct RAM* memory, char* name)
{
	// nothing to search 
	if (memory == NULL || name == NULL) {
		return NULL;
	}

	// get address corresponding to identifier/name
	int addr = ram_get_addr(memory, name);

	// make sure its a valid address
	if (addr == -1) {
		return NULL;
	}

	// read cell by address
	return ram_read_cell_by_addr(memory, addr);
}


//
// ram_free_value
//
// Frees the memory value returned by ram_read_cell_by_name and
// ram_read_cell_by_addr.
//
void ram_free_value(struct RAM_VALUE* value)
{
	// nothing to free
	if (value == NULL) {
		return;
	}

	// if value is a string, we need to free its duplicate
	if (value->value_type == RAM_TYPE_STR && value->types.s != NULL) {
		free(value->types.s);
		value->types.s = NULL;
	}

	// free value itself
	free(value);
}


//
// ram_write_cell_by_addr
//
// Writes the given value to the memory cell at the given 
// address. If a value already exists at this address, that
// value is overwritten by this new value. Returns true if 
// the value was successfully written, false if not (which 
// implies the memory address is invalid).
// 
// NOTE: if the value being written is a string, it will
// be duplicated and stored.
// 
// NOTE: a variable has to be written to memory before its
// address becomes valid. Once a variable is written to memory,
// its address never changes.
//
bool ram_write_cell_by_addr(struct RAM* memory, struct RAM_VALUE value, int address)
{
	// nothing to write/can't write var & check validity of address
	if (memory == NULL || address < 0 || address >= memory->num_values || &value == NULL ) {
		return false;
	}

	struct RAM_CELL* cell = &memory->cells[address];

	// geet rid of existing data if a string is there
	if (cell->value.value_type == RAM_TYPE_STR && cell->value.types.s != NULL) {
		free(cell->value.types.s);
		cell->value.types.s = NULL;
	}

	cell->value.value_type = value.value_type;

	switch (value.value_type) {
		case RAM_TYPE_INT:
		case RAM_TYPE_PTR:
		case RAM_TYPE_BOOLEAN:
			cell->value.types.i = value.types.i;
			break;

		case RAM_TYPE_REAL:
			cell->value.types.d = value.types.d;
			break;

		case RAM_TYPE_STR:
			if (value.types.s != NULL) {
				cell->value.types.s = strdup(value.types.s);
			if (cell->value.types.s == NULL) {
				return false;  // strdup failed
			}
			} else {
				cell->value.types.s = NULL;
			}
			break;

		case RAM_TYPE_NONE:
			// don't do anything for None
			break;

		default:
			return false;  // unknown var type
	}
	return true;
}


//
// ram_write_cell_by_name
//
// Writes the given value to a memory cell named by the given
// name. If a memory cell already exists with this name, the
// existing value is overwritten by the given value. Returns
// true since this operation always succeeds.
// 
// NOTE: if the value being written is a string, it will
// be duplicated and stored.
// 
// NOTE: a variable has to be written to memory before its
// address becomes valid. Once a variable is written to memory,
// its address never changes.
//
bool ram_write_cell_by_name(struct RAM* memory, struct RAM_VALUE value, char* name)
{
	// nothing to write/invalid name
	if (memory == NULL || name == NULL || &value == NULL) {
		return false;
	}

	// try to get address of var based on name
	// if we get it, just use other function
	int addr = ram_get_addr(memory, name);
	if (addr != -1) {
		return ram_write_cell_by_addr(memory, value, addr);
	}

	// if we get here, that means it doesn't exist and must add a new cell
	// are we at capacity?
	if (memory->num_values >= memory->capacity) {
		int new_cap = memory->capacity * 2;
		struct RAM_CELL* new_cells = (struct RAM_CELL*)realloc(memory->cells, new_cap * sizeof(struct RAM_CELL));
		if (new_cells == NULL) {
			return false; 	// reallocation of memory failed somehow
		}

		memory->cells = new_cells;
		memory->capacity = new_cap;

		// initialize all new cells to default values of None
		for (int i = memory->num_values; i < memory->capacity; i++) {
			memory->cells[i].identifier = NULL;
			memory->cells[i].value.value_type = RAM_TYPE_NONE;
		}
	}

	// initialize the new cell
	struct RAM_CELL* cell = &memory->cells[memory->num_values];
	cell->identifier = strdup(name);

	if (cell->identifier == NULL) {
		return false;
	}

	cell->value.value_type = value.value_type;

	switch (value.value_type) {
		case RAM_TYPE_INT:
		case RAM_TYPE_PTR:
		case RAM_TYPE_BOOLEAN:
			cell->value.types.i = value.types.i;
			break;

		case RAM_TYPE_REAL:
			cell->value.types.d = value.types.d;
			break;

		case RAM_TYPE_STR:
			if (value.types.s != NULL) {
				cell->value.types.s = strdup(value.types.s);
			if (cell->value.types.s == NULL) {
				free(cell->identifier);  // free if fails
				cell->identifier = NULL;
				return false;  
			}
			} else {
				cell->value.types.s = NULL;
			}
			break;

		case RAM_TYPE_NONE:
			// don't do anything for None type
			break;

		default:
			free(cell->identifier);  // free if unknown type
			cell->identifier = NULL;
			return false; 
	}
	memory->num_values++;
	return true;
}


//
// ram_print
//
// Prints the contents of memory to the console.
//
void ram_print(struct RAM* memory)
{
	printf("**MEMORY PRINT**\n");

	if (memory == NULL) {
		printf("**MEMORY PRINT**\n");
		printf("Memory is NULL\n");
		printf("**END PRINT**\n");
		return;
	}

	printf("**MEMORY PRINT**\n");
	printf("Capacity: %d\n", memory->capacity);
	printf("Num values: %d\n", memory->num_values);
	printf("Contents:\n");

	for (int i = 0; i < memory->num_values; i++) {
		struct RAM_CELL* cell = &memory->cells[i];

		printf(" %d: ", i);
		if (cell->identifier != NULL) {
			printf("%s, ", cell->identifier);
		} else {
			printf("<no identifier>, ");
		}

		// Print value based on type
		switch (cell->value.value_type) {
			case RAM_TYPE_INT:
				printf("int, %d", cell->value.types.i);
				break;
			case RAM_TYPE_REAL:
				printf("real, %lf", cell->value.types.d);
				break;
			case RAM_TYPE_STR:
				if (cell->value.types.s != NULL) {
					printf("str, '%s'", cell->value.types.s);
				} else {
					printf("str, <null>");
				}
				break;
			case RAM_TYPE_PTR:
				printf("ptr, %d", cell->value.types.i);  
				break;
			case RAM_TYPE_BOOLEAN:
				printf("boolean, %s", cell->value.types.i ? "True" : "False");
				break;
			case RAM_TYPE_NONE:
				printf("none, None");
				break;
			default:
				printf("unknown type");
				break;
		}
		printf("\n");
	}
	printf("**END PRINT**\n");
}
