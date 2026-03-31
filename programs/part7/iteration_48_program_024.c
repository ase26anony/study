/* types_basic.h - Basic type declarations for gengtype coverage */

#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H

/* TYPE_SCALAR: Basic built-in types */
int global_int;
char global_char;
double global_double;
long long global_longlong;
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
float float_array[20];

/* TYPE_POINTER: Pointer declarations */
int *int_ptr;
char **char_ptr_ptr;
void *void_ptr;
double *double_ptr_array[5];

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct incomplete_struct;
union incomplete_union;
enum incomplete_enum;
struct undefined_type;

#endif /* TYPES_BASIC_H */
