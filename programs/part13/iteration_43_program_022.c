/* test-coverage.gt - Comprehensive test file for gengtype coverage */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef int scalar_int;
typedef double scalar_double;
enum my_enum { E1, E2, E3 };

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char *, void *);

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef const char *string_array[5];

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
    int field1;
    double field2;
    char field3;
};

/* TYPE_USER_STRUCT: GC-aware struct with user tag */
struct GTY((user)) user_struct {
    void *data;
    int id;
};

/* TYPE_UNION: Union type */
union my_union {
    int a;
    void *b;
    double c;
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
    void *ptr_field;
    
    /* TYPE_ARRAY (fixed size) */
    int array_field[20];
    
    /* TYPE_STRING */
    const char *name;
    
    /* TYPE_SCALAR */
    int count;
    enum my_enum enum_field;
    
    /* TYPE_CALLBACK */
    callback_type callback;
    
    /* Nested TYPE_STRUCT */
    struct plain_struct nested_plain;
    
    /* Pointer to TYPE_UNION */
    union my_union *union_ptr;
    
    /* TYPE_ARRAY of pointers */
    struct opaque_struct *opaque_array[5];
    
    /* Flexible array member (variable-length array) */
    int flexible_array[];
};

/* Another GC struct with chain of pointers */
struct GTY(()) linked_node {
    int value;
    
    /* TYPE_POINTER to same type (linked list) */
    struct linked_node * GTY((skip)) next;
    
    /* TYPE_POINTER to different type */
    struct complex_struct *complex_data;
    
    /* TYPE_ARRAY of strings */
    const char *tags[10];
};

/* Union containing GC pointers */
union GTY(()) gc_union {
    struct linked_node * GTY((tag("0"))) as_node;
    struct complex_struct * GTY((tag("1"))) as_complex;
    void * GTY((tag("2"))) as_opaque;
};

/* Struct with callback field */
struct GTY(()) has_callbacks {
    /* TYPE_CALLBACK */
    int (*compare_func)(const void *, const void *);
    
    /* Array of callbacks */
    callback_type handlers[5];
    
    /* String array */
    const char *messages[];
};

/* Opaque pointer type for TYPE_UNDEFINED testing */
typedef struct undefined_type *undefined_ptr;

/* Mix of typedefs for various types */
typedef union my_union union_alias;
typedef struct plain_struct struct_alias;
typedef int (*complex_callback)(struct complex_struct *, union my_union);

/* Additional scalar types */
typedef unsigned long long scalar_ull;
typedef _Bool scalar_bool;
typedef float scalar_float;

/* Array of unions */
union my_union union_array[100];

/* Struct with bitfields (scalar handling) */
struct with_bitfields {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int value : 8;
};

/* Const pointer types */
typedef int * const const_ptr;
typedef const struct complex_struct *const_complex_ptr;

/* Void pointer array */
void *void_ptr_array[50];

/* Nested array type */
typedef int matrix[10][10];

/* Function pointer with complex signature */
typedef void (*signal_handler)(int signum, void *user_data, const char *message);

/* Final catch-all structure with every type */
struct GTY(()) all_types {
    /* TYPE_SCALAR */
    int scalar_int;
    double scalar_double;
    enum my_enum scalar_enum;
    
    /* TYPE_STRING */
    const char *string_field;
    
    /* TYPE_POINTER */
    void *pointer_field;
    struct all_types *self_pointer;
    
    /* TYPE_ARRAY */
    int int_array[100];
    const char *string_array_field[20];
    
    /* TYPE_CALLBACK */
    callback_type callback_field;
    signal_handler signal_callback;
    
    /* TYPE_STRUCT (nested) */
    struct plain_struct nested_struct;
    
    /* TYPE_UNION */
    union my_union nested_union;
    
    /* Pointer to TYPE_USER_STRUCT */
    struct user_struct *user_struct_ptr;
    
    /* Pointer to TYPE_LANG_STRUCT */
    struct lang_specific *lang_struct_ptr;
    
    /* TYPE_ARRAY of unions */
    union my_union union_array_field[10];
    
    /* Flexible array of pointers */
    void *flexible_pointer_array[];
};
