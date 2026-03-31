#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct incomplete_struct;
union incomplete_union;
enum incomplete_enum;

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
const char *global_string = "Hello, World!";
extern char error_string[];
char *another_string = "Another string";
const char *const constant_string = "Constant";

/* TYPE_ARRAY: Array declarations */
int int_array[10];
double double_array[5][5];
char char_array[20];
float multi_dim[2][3][4];
char *string_array[5];

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
double *double_ptr;
char **string_ptr_ptr;
void *void_ptr;
const int *const_ptr;

#endif /* TYPES_BASIC_H */
