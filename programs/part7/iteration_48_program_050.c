/* types_basic.h - Basic type declarations for gengtype coverage */

#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H

/* TYPE_SCALAR: Basic built-in types */
int global_int;
char global_char;
double global_double;
long long global_long_long;
_Bool global_bool;
short global_short;
unsigned int global_uint;
float global_float;

/* TYPE_STRING: String types */
const char *global_string = "Hello, world!";
extern char error_string[];
char *another_string = "test";

/* TYPE_ARRAY: Array declarations */
int int_array[10];
double matrix[5][5];
char *string_array[3];
float multi_dim[2][3][4];

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
char **char_ptr_ptr;
void *void_ptr;
double *double_ptr;

/* Function pointer (will be TYPE_CALLBACK when properly used) */
void (*simple_func_ptr)(void);

#endif /* TYPES_BASIC_H */
