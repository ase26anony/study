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
float global_float;
unsigned int global_uint;
signed char global_schar;

/* TYPE_STRING declarations */
const char *global_string = "Hello, World!";
extern char error_string[];
char *messages[] = {"msg1", "msg2", "msg3"};
const char *const constant_string = "Constant";

/* TYPE_ARRAY declarations */
int int_array[10];
double matrix[5][5];
char char_matrix[3][4][5];
float float_array[100];
long double long_double_array[20];

/* TYPE_POINTER declarations */
int *int_ptr;
char **string_ptr_ptr;
void *void_ptr;
const int *const_int_ptr;
volatile char *volatile_char_ptr;

/* Function pointer arrays (TYPE_ARRAY + TYPE_CALLBACK) */
int (*func_ptr_array[5])(int, int);
void (*void_func_array[3])(void);

#endif /* TYPES_BASIC_H */
