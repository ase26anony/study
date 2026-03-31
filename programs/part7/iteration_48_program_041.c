/* types_basic.h - Basic type declarations for gengtype coverage */

#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H

/* TYPE_SCALAR examples */
int global_int;
char global_char;
double global_double;
long long global_long_long;
_Bool global_bool;
unsigned int global_uint;
float global_float;

/* TYPE_STRING examples */
const char *error_message = "Error occurred";
extern char version_string[];
char *welcome_msg = "Welcome";
static const char *static_str = "Static string";

/* TYPE_ARRAY examples */
int int_array[10];
double matrix[5][5];
char *string_array[3];
float multi_dim[2][3][4];
long long big_array[100];

/* TYPE_POINTER examples */
int *int_ptr;
char **char_ptr_ptr;
void *void_ptr;
double *double_ptr;
int (*array_ptr)[10];
void (*simple_func_ptr)(void);

/* TYPE_CALLBACK examples */
typedef int (*comparator_t)(const void *, const void *);
comparator_t compare_func;

void (*signal_handler)(int);
int (*math_operation)(int, int);

/* Nested combinations */
int (*array_of_func_ptrs[5])(int, int);  /* TYPE_ARRAY + TYPE_CALLBACK */
char *(*string_processor)(const char *);

#endif /* TYPES_BASIC_H */
