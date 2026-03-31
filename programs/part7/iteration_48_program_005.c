/* types_basic.h - Basic type declarations for gengtype coverage */

#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H

/* TYPE_SCALAR: Basic built-in types */
int global_int;
char global_char;
double global_double;
long long global_long_long;
_Bool global_bool;
unsigned int global_uint;
float global_float;
short global_short;
signed char global_schar;

/* TYPE_STRING: String types */
const char *global_string = "Hello, World!";
extern char error_string[];
char *messages[] = {"msg1", "msg2", "msg3"};

/* TYPE_ARRAY: Array declarations */
int int_array[10];
double matrix[5][5];
char char_matrix[3][4][5];
float float_array[100];

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
char **char_ptr_ptr;
void *void_ptr;
const int *const_int_ptr;
volatile char *volatile_char_ptr;

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct incomplete_struct;
union unknown_union;
enum undefined_enum;

#endif /* TYPES_BASIC_H */
