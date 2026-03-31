/* test-coverage.gt - Comprehensive test file for gengtype coverage */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef int scalar_int;
typedef float scalar_float;
enum my_enum { E1, E2, E3 };

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char *);

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_ARRAY: Array types */
int fixed_array[10];
typedef char char_array[20];

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
    int field1;
    float field2;
    char *field3;
};

/* TYPE_USER_STRUCT: GC-aware struct with user tag */
struct GTY((user)) user_struct {
    void *data;
    int id;
};

/* TYPE_UNION: Union type */
union my_union {
    int a;
    float b;
    void *c;
    callback_type d;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void *lang_data;
};

/* Complex nested types to ensure deep processing */

/* A GC-tracked struct containing various type combinations */
struct GTY(()) complex_struct {
    /* TYPE_POINTER */
    void *ptr;
    
    /* TYPE_ARRAY (flexible array member) */
    int flexible_array[] GTY((length("count")));
    
    /* TYPE_STRING */
    const char *name;
    
    /* TYPE_SCALAR */
    int count;
    enum my_enum status;
    
    /* TYPE_CALLBACK */
    callback_type handler;
    
    /* TYPE_UNION */
    union my_union data;
    
    /* TYPE_POINTER to another struct */
    struct plain_struct *plain;
    
    /* TYPE_POINTER to user struct */
    struct user_struct *user;
    
    /* TYPE_POINTER to lang struct */
    struct lang_specific *lang;
};

/* Another union with GTY marker */
union GTY(()) tagged_union {
    struct complex_struct *cs;
    struct user_struct *us;
    int value;
    callback_type cb;
};

/* Array of pointers */
struct GTY(()) array_container {
    /* TYPE_ARRAY of pointers */
    struct complex_struct *items[10];
    
    /* TYPE_ARRAY of scalars */
    int indices[20];
    
    /* TYPE_STRING array */
    const char *names[5];
};

/* Struct with callback array */
struct GTY(()) callback_container {
    /* TYPE_ARRAY of callbacks */
    callback_type handlers[8];
    
    /* TYPE_POINTER to array */
    int (*matrix)[4][4];
};

/* Opaque pointer type for TYPE_UNDEFINED testing */
typedef struct undefined_type *undefined_ptr;

/* Mix of typedefs for various types */
typedef union my_union union_alias;
typedef struct plain_struct struct_alias;
typedef int (*complex_callback)(struct complex_struct *, union my_union);

/* Additional test for nested arrays */
struct GTY(()) nested_arrays {
    int matrix[3][3];
    char *string_array[10];
    callback_type callback_matrix[2][2];
};

/* Test for zero-length array */
struct GTY(()) zero_length {
    int count;
    char data[0];
};

/* Test for variable-length array in struct */
struct GTY(()) var_length {
    int length;
    int array[1];
};

/* Additional scalar types */
typedef _Bool bool_type;
typedef long long int64_type;
typedef double double_type;
typedef long double long_double_type;

/* Pointer to array */
typedef int (*array_ptr)[10];

/* Function returning pointer */
typedef struct complex_struct *(*factory_func)(int);

/* Const pointer types */
typedef const int *const_int_ptr;
typedef struct complex_struct *const const_struct_ptr;

/* Anonymous struct/union in GTY */
struct GTY(()) anonymous_container {
    struct {
        int x;
        int y;
    } point;
    
    union {
        int i;
        float f;
    } value;
};

/* For testing TYPE_NONE - this should never be reached in normal execution,
   but we include various edge cases to ensure all paths are potentially reachable */

/* Include some preprocessor conditions to test different code paths */
#ifdef SPECIAL_CASE
struct GTY(()) special_struct {
    /* This might trigger different code paths */
    void *special_data;
};
#endif

/* Multiple levels of indirection */
typedef struct complex_struct **double_ptr;
typedef int ***triple_int_ptr;

/* Mixed declarations */
static int static_scalar = 42;
const int const_scalar = 100;
volatile int volatile_scalar = 200;

/* External declarations */
extern struct complex_struct *external_ptr;
extern const char *external_string;

/* Complete the forward declaration from the beginning */
struct opaque_struct {
    int defined_now;
    void *data;
};

/* Final test: struct containing all type categories */
struct GTY(()) ultimate_test {
    /* TYPE_SCALAR */
    int id;
    enum my_enum type;
    
    /* TYPE_STRING */
    const char *description;
    
    /* TYPE_POINTER */
    void *generic_ptr;
    struct ultimate_test *self_ptr;
    
    /* TYPE_ARRAY */
    int numbers[5];
    char name[32];
    
    /* TYPE_UNION */
    union my_union data_union;
    
    /* TYPE_CALLBACK */
    callback_type notify;
    
    /* TYPE_STRUCT (embedded) */
    struct plain_struct embedded;
    
    /* TYPE_USER_STRUCT (pointer) */
    struct user_struct *user_data;
    
    /* TYPE_LANG_STRUCT (pointer) */
    struct lang_specific *lang_data;
    
    /* Flexible array member */
    char extra_data[];
};
