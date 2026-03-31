/* types_basic.h - Basic type declarations for gengtype coverage */

#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct incomplete_struct;
union incomplete_union;

/* TYPE_SCALAR: Multiple scalar types grouped together */
int scalar_int;
char scalar_char;
double scalar_double;
long long scalar_long_long;
_Bool scalar_bool;
float scalar_float;
short scalar_short;
unsigned int scalar_unsigned;

/* TYPE_STRING: String declarations */
const char *string_literal = "test string";
extern char error_string[];
char *another_string;
const char *const constant_string = "constant";

/* TYPE_ARRAY: Various array declarations */
int int_array[10];
double double_array[5][5];
char char_array[3][4][5];
float float_array[20];
long long multi_dim_array[2][3][4][5];

/* TYPE_POINTER: Various pointer declarations */
int *int_pointer;
double **double_double_pointer;
char ***triple_char_pointer;
void *void_pointer;
const int *const_int_pointer;
volatile char *volatile_char_pointer;

#endif /* TYPES_BASIC_H */
