#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct incomplete;           /* Line 1: TYPE_UNDEFINED */
union unknown;               /* Line 2: TYPE_UNDEFINED */
struct another_incomplete;   /* Line 3: TYPE_UNDEFINED */

/* TYPE_SCALAR: Basic built-in types */
int global_int;              /* Line 4: TYPE_SCALAR */
char global_char;            /* Line 5: TYPE_SCALAR */
double global_double;        /* Line 6: TYPE_SCALAR */
long long global_longlong;   /* Line 7: TYPE_SCALAR */
_Bool global_bool;           /* Line 8: TYPE_SCALAR */
float global_float;          /* Line 9: TYPE_SCALAR */
short global_short;          /* Line 10: TYPE_SCALAR */
unsigned int global_uint;    /* Line 11: TYPE_SCALAR */

/* TYPE_STRING: String types */
const char *error_message = "Error occurred";  /* Line 12: TYPE_STRING */
extern char version_string[];                  /* Line 13: TYPE_STRING */
char *dynamic_string;                          /* Line 14: TYPE_STRING */

/* TYPE_ARRAY: Array declarations */
int int_array[10];                            /* Line 15: TYPE_ARRAY */
double matrix[5][5];                          /* Line 16: TYPE_ARRAY */
char *string_array[3];                        /* Line 17: TYPE_ARRAY */
long multi_dim[2][3][4];                      /* Line 18: TYPE_ARRAY */

/* TYPE_POINTER: Various pointer types */
int *int_ptr;                                 /* Line 19: TYPE_POINTER */
void *void_ptr;                               /* Line 20: TYPE_POINTER */
char **char_ptr_ptr;                          /* Line 21: TYPE_POINTER */

#endif /* TYPES_BASIC_H */
