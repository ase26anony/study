#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct incomplete_struct;      /* Will trigger TYPE_UNDEFINED */
union incomplete_union;        /* Will trigger TYPE_UNDEFINED */
enum incomplete_enum;          /* Will trigger TYPE_UNDEFINED */

/* TYPE_SCALAR: Basic built-in types */
int scalar_int;                /* TYPE_SCALAR */
char scalar_char;              /* TYPE_SCALAR */
double scalar_double;          /* TYPE_SCALAR */
long long scalar_long_long;    /* TYPE_SCALAR */
_Bool scalar_bool;             /* TYPE_SCALAR */
float scalar_float;            /* TYPE_SCALAR */
short scalar_short;            /* TYPE_SCALAR */
unsigned int scalar_unsigned;  /* TYPE_SCALAR */

/* TYPE_STRING: String types */
const char *string_literal = "Hello, World!";  /* TYPE_STRING */
extern char error_string[];                     /* TYPE_STRING */
char *another_string;                           /* TYPE_STRING */

/* TYPE_ARRAY: Array declarations */
int int_array[10];                              /* TYPE_ARRAY */
double matrix[5][5];                            /* TYPE_ARRAY (2D) */
char *string_array[3];                          /* TYPE_ARRAY of pointers */
float multi_dim[2][3][4];                       /* TYPE_ARRAY (3D) */

/* TYPE_POINTER: Simple pointer declarations */
int *int_ptr;                                   /* TYPE_POINTER */
void *void_ptr;                                 /* TYPE_POINTER */
char **double_ptr;                              /* TYPE_POINTER to pointer */

#endif /* TYPES_BASIC_H */
