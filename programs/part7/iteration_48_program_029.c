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
char *messages[] = {"msg1", "msg2", "msg3"};

/* TYPE_ARRAY: Array declarations */
int int_array[10];
double matrix[5][5];
char char_matrix[3][4][5];
long *ptr_array[8];

/* TYPE_POINTER: Pointer declarations */
int *int_ptr;
double *double_ptr;
char **string_ptr_ptr;
void *void_ptr;
const volatile int *cv_int_ptr;

/* Function pointer (TYPE_CALLBACK) */
void (*simple_callback)(void);
int (*math_func)(int, int);

#endif /* TYPES_BASIC_H */
