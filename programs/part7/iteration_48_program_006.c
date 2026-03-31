/* types_basic.h - Basic type declarations for gengtype coverage */

#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H

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
const char *global_string = "Hello, World!";
extern char error_string[];
char *messages[] = {"msg1", "msg2", "msg3"};

/* TYPE_ARRAY: Array declarations */
int int_array[10];
double matrix[5][5];
char char_matrix[3][4][5];
float float_array[20];

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
double *double_ptr;
char **string_ptr_ptr;
void *void_ptr;
const void *const_void_ptr;

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void *, const void *);
void (*signal_handler)(int);
int (*math_operation)(int, int);

/* Nested callback in struct */
struct CallbackHolder {
    void (*callback)(void *data);
    int (*filter)(int);
};

#endif /* TYPES_BASIC_H */
