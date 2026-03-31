/* types_basic.h - Basic type declarations for gengtype coverage */

#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct incomplete_struct;
union incomplete_union;

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

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
char **char_ptr_ptr;
void *void_ptr;
const void *const_void_ptr;
volatile int *volatile_int_ptr;

/* TYPE_ARRAY: Array declarations */
int int_array[10];
double double_array[5][5];
char char_2d_array[3][4];
long *ptr_array[8];
int (*array_of_func_ptrs[5])(void);

#endif /* TYPES_BASIC_H */
