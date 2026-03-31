/* types_basic.h - Basic type declarations for gengtype coverage */

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
extern char version_string[];
char *welcome_msg = "Welcome";
static const char *static_string = "Static string";

/* TYPE_ARRAY declarations */
int int_array[10];
double matrix[5][5];
char *string_array[3];
float multi_dim[2][3][4];

/* TYPE_POINTER declarations */
int *int_ptr;
char **char_ptr_ptr;
void *void_ptr;
double *double_ptr;

/* TYPE_UNDEFINED forward declarations */
struct incomplete_struct;
union unknown_union;
enum undefined_enum;

#endif /* TYPES_BASIC_H */
