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
float global_float;
unsigned int global_uint;

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
const void *const_void_ptr;

/* TYPE_CALLBACK: Function pointers */
void (*simple_callback)(void);
int (*math_func)(int, int);
char *(*string_func)(const char *);
void (*complex_callback)(int, double, char*);

/* Array of function pointers (triggers TYPE_ARRAY and TYPE_CALLBACK) */
int (*operation_array[5])(int, int);
void (*handler_array[10])(void);

#endif /* TYPES_BASIC_H */
