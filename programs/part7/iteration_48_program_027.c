/* types_basic.h - Basic scalar, string, array, and pointer types */

#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H

#include <stddef.h>

/* TYPE_SCALAR declarations (grouped) */
int global_int;
char global_char;
double global_double;
long long global_long_long;
_Bool global_bool;
short global_short;
float global_float;
unsigned int global_uint;

/* TYPE_STRING declarations */
const char *error_message = "Error occurred";
extern char system_string[];
char *dynamic_string = "Dynamic";
static const char *static_msg = "Static message";

/* TYPE_ARRAY declarations (grouped) */
int int_array[10];
double matrix[5][5];
char *string_array[3];
float three_d_array[3][3][3];
long multi_array[2][4][6][8];

/* TYPE_POINTER declarations (grouped) */
int *int_ptr;
char **char_ptr_ptr;
void *void_ptr;
double *double_ptr_array[5];
int (*array_of_ptrs)[10];
void (*simple_func_ptr)(void);

#endif /* TYPES_BASIC_H */
