/* types_basic.h - Basic type declarations for gengtype coverage */

#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H

/* TYPE_SCALAR: Basic built-in types */
int global_int;
char global_char;
double global_double;
long long global_long_long;
_Bool global_bool;
float global_float;
short global_short;
unsigned int global_uint;

/* TYPE_STRING: String types */
const char *global_string = "Hello, world!";
extern char error_string[];
char *messages[] = {"msg1", "msg2", "msg3"};

/* TYPE_ARRAY: Array declarations */
int int_array[10];
double matrix[5][5];
char char_matrix[3][4][5];
float float_array[100];

/* TYPE_POINTER: Pointer declarations */
int *int_ptr;
char **char_ptr_ptr;
void *void_ptr;
double *double_ptr_array[5];

/* TYPE_CALLBACK: Function pointers */
void (*simple_callback)(void);
int (*math_func)(int, int);
char *(*string_processor)(const char*);

/* Nested callback in array */
int (*callbacks[5])(int, int);

#endif /* TYPES_BASIC_H */
