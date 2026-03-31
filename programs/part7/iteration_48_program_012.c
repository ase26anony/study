#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H

/* TYPE_SCALAR declarations */
int global_int;
char global_char;
double global_double;
long long global_long_long;
_Bool global_bool;
float global_float;
short global_short;
unsigned int global_uint;

/* TYPE_STRING declarations */
const char *error_message = "Error occurred";
extern char system_string[];
char *welcome_msg = "Welcome";
const char *const constant_string = "Constant";

/* TYPE_ARRAY declarations */
int integer_array[100];
double matrix[10][10];
char *string_array[50];
float three_d_array[5][5][5];
long multi_array[2][3][4][5];

/* TYPE_POINTER declarations */
int *int_ptr;
char **string_ptr_ptr;
void *generic_ptr;
double *double_ptr_array[10];
int (*array_of_int_ptrs)[20];

#endif /* TYPES_BASIC_H */
