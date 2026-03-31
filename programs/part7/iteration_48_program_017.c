/* types_basic.h - Basic type declarations for gengtype coverage */

#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H

/* TYPE_SCALAR declarations */
int global_int;
char global_char;
double global_double;
long long global_long_long;
_Bool global_bool;
short global_short;
unsigned int global_uint;
float global_float;

/* TYPE_STRING declarations */
const char *global_string = "Hello, world!";
extern char error_string[];
char *messages[] = {"msg1", "msg2", "msg3"};

/* TYPE_ARRAY declarations */
int int_array[10];
double matrix[5][5];
char char_matrix[3][4][5];
float float_array[20];

/* TYPE_POINTER declarations */
int *int_ptr;
char **char_ptr_ptr;
void *void_ptr;
double *double_ptr;

/* Function pointer (TYPE_CALLBACK) */
void (*simple_callback)(void);
int (*math_func)(int, int);

#endif /* TYPES_BASIC_H */
