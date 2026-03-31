/* types_basic.h - Covering basic type categories */

/* TYPE_SCALAR: Basic built-in types */
int global_int;
char global_char;
double global_double;
long long global_long_long;
_Bool global_bool;
unsigned int global_uint;
float global_float;

/* TYPE_STRING: String types */
const char *global_string = "Hello, World!";
extern char error_string[];
char *messages[] = {"msg1", "msg2", "msg3"};

/* TYPE_ARRAY: Various array declarations */
int int_array[10];
double matrix[5][5];
char char_matrix[3][4][5];
float *pointer_array[8];

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
char **char_ptr_ptr;
void *void_ptr;
const void *const_void_ptr;

/* Nested: Array of pointers */
int *ptr_array[10];
char *string_array[5];

/* Function pointer (will be TYPE_CALLBACK when properly defined) */
typedef int (*compare_func_t)(const void *, const void *);
