/* types_basic.h - Basic type declarations for gengtype coverage */

#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H

/* TYPE_SCALAR declarations */
int global_int;
char global_char;
double global_double;
long long global_longlong;
_Bool global_bool;
float global_float;
short global_short;
unsigned int global_uint;

/* TYPE_STRING declarations */
const char *error_message = "Error occurred";
extern char version_string[];
char *welcome_msg = "Welcome";
const char *const constant_string = "Constant";

/* TYPE_ARRAY declarations */
int int_array[10];
double matrix[5][5];
char *string_array[3];
float three_d_array[2][3][4];
long multi_array[2][3];

/* TYPE_POINTER declarations */
int *int_ptr;
char **string_ptr_ptr;
void *void_ptr;
double *double_ptr_array[5];
struct Point *struct_ptr;  /* Forward reference */

/* TYPE_UNDEFINED declarations */
struct incomplete_struct;
union undefined_union;
enum unknown_enum;

#endif /* TYPES_BASIC_H */
