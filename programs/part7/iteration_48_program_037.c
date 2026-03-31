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
const char *global_string = "Hello, World!";
extern char error_string[];
char *messages[] = {"msg1", "msg2", "msg3"};

/* TYPE_ARRAY: Array declarations */
int int_array[10];
double matrix[5][5];
char char_matrix[3][4][5];
float float_array[20];

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
char **char_ptr_ptr;
void *void_ptr;
const int *const_int_ptr;
volatile char *volatile_char_ptr;

/* Nested combinations */
int *pointer_array[10];           /* Array of pointers */
int (*array_pointer)[5];          /* Pointer to array */
int (*func_ptr_array[3])(void);   /* Array of function pointers */

#endif /* TYPES_BASIC_H */
