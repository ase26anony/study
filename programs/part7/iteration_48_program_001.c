/* types_basic.h - Covering TYPE_SCALAR, TYPE_STRING, TYPE_ARRAY */

#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H

/* TYPE_SCALAR declarations - group multiple to hit same case */
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
const char *error_message = "Error occurred";
extern char version_string[];
char *welcome_msg = "Welcome";
static const char *static_string = "Static string";

/* TYPE_ARRAY declarations */
int int_array[10];
double matrix[5][5];
char *string_array[3];
float multi_dim[2][3][4];
long coordinates[100];

/* Mixed scalar and array */
unsigned char byte_array[256];
_Bool flags[8];

#endif /* TYPES_BASIC_H */
