/* types_basic.h - Basic types for gengtype coverage */

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct incomplete_struct;
union incomplete_union;
enum incomplete_enum;

/* TYPE_SCALAR: Basic built-in types */
typedef int scalar_int;
typedef char scalar_char;
typedef double scalar_double;
typedef long long scalar_long_long;
typedef _Bool scalar_bool;
typedef float scalar_float;
typedef short scalar_short;
typedef unsigned int scalar_uint;
typedef unsigned char scalar_uchar;
typedef signed char scalar_schar;

/* TYPE_STRING: String types */
extern const char *error_message;
extern char version_string[];
static const char *static_string = "Hello, World!";
char *dynamic_string;
const char *const constant_string = "Constant";

/* TYPE_ARRAY: Array declarations */
int int_array[10];
double double_array[5][5];
char char_array[3][4][5];
float *pointer_array[8];
const int const_array[3] = {1, 2, 3};

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
double **double_ptr_ptr;
void *void_ptr;
const void *const_void_ptr;
volatile int *volatile_int_ptr;
