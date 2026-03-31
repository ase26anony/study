#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct *undefined_ptr_t;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef char scalar_char;
typedef long scalar_long;
typedef short scalar_short;
typedef unsigned int scalar_uint;
typedef _Bool scalar_bool;

/* TYPE_STRING: String types and literals */
#define STRING_LITERAL "This is a string literal"
const char *string_pointer = "Constant string pointer";
char string_array[] = "String array";
const char *const string_const_ptr = "Const pointer to const string";

/* TYPE_STRUCT: Complete struct types with mixed members */
struct simple_struct {
    scalar_int id;
    scalar_float value;
    char name[32];
};

struct complex_struct {
    struct simple_struct base;
    scalar_double *data_ptr;
    int array[10];
    struct complex_struct *next;
};

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct {
    scalar_int x;
    scalar_double y;
    char label[64];
} user_struct_t;

typedef struct complex_struct complex_struct_alias;

/* TYPE_UNION: Union types with various members */
union simple_union {
    scalar_int as_int;
    scalar_float as_float;
    scalar_double as_double;
    char as_char[8];
};

union nested_union {
    struct {
        scalar_int type;
        union simple_union data;
    } tagged;
    struct complex_struct as_struct;
    user_struct_t as_user;
};

/* GCC-specific union attribute */
union transparent_union_example {
    int *int_ptr;
    long *long_ptr;
    void *void_ptr;
} __attribute__((transparent_union));

/* TYPE_POINTER: Pointers to all previously defined types */
typedef scalar_int *scalar_ptr;
typedef struct simple_struct *struct_ptr;
typedef union simple_union *union_ptr;
typedef user_struct_t *user_struct_ptr;
typedef void (*func_ptr)(void);
typedef const char *const *const_string_ptr_ptr;

/* TYPE_ARRAY: Arrays of different dimensions and element types */
scalar_int scalar_array_1d[100];
scalar_double scalar_array_2d[10][20];
struct simple_struct struct_array[50];
user_struct_t user_struct_array[25][4];
union simple_union union_array[30];
void *pointer_array[40];

/* Multi-dimensional complex array */
typedef struct complex_struct complex_array_t[5][5][5];

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef scalar_int (*callback_int_int)(scalar_int);
typedef scalar_double (*callback_double_double)(scalar_double, scalar_double);
typedef void (*callback_void_struct)(const struct simple_struct*);
typedef user_struct_t *(*callback_user_ptr)(scalar_int, const char*);
typedef scalar_int (*callback_complex)(struct complex_struct*, 
                                      user_struct_t**, 
                                      union simple_union*, 
                                      scalar_double);

/* Struct containing callback members */
struct callback_container {
    callback_int_int int_handler;
    callback_void_struct struct_handler;
    callback_complex complex_handler;
    void (*generic_handler)(void*);
};

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */
/* These are recognized by gengtype as language-specific types */
struct tree_common;
struct tree_decl_common;
struct tree_type_common;

/* Dummy structures with GCC internal naming patterns */
struct lang_type {
    struct tree_type_common *type;
    void *lang_specific;
};

struct lang_decl {
    struct tree_decl_common *decl;
    unsigned lang_flag : 1;
};

/* Complex nested type hierarchy */
typedef struct node {
    scalar_int value;
    struct node *left;
    struct node *right;
    void (*traverse)(struct node*, void (*)(scalar_int));
} tree_node_t;

/* Ultimate nested type combining everything */
typedef struct ultimate_type {
    /* Scalar members */
    scalar_int id;
    scalar_double precision;
    
    /* String member */
    const char *type_name;
    
    /* Struct member */
    struct simple_struct base_data;
    
    /* User struct member */
    user_struct_t user_data;
    
    /* Union member */
    union nested_union variant;
    
    /* Pointer members */
    struct ultimate_type *self_ptr;
    void **generic_ptrs;
    
    /* Array members */
    scalar_int matrix[4][4];
    struct callback_container handlers[3];
    
    /* Callback members */
    callback_complex compute;
    scalar_int (*validate)(struct ultimate_type*);
    
    /* Language struct pointer */
    struct lang_type *lang_info;
    
    /* Nested struct with array of pointers to unions */
    struct {
        int count;
        union simple_union *items[10];
        void (*process)(union simple_union**, int);
    } collection;
    
    /* Function pointer returning pointer to struct containing callback */
    struct callback_container *(*get_handler_container)(scalar_int);
    
    /* Attribute to influence gengtype processing */
    scalar_char padding[16];
} __attribute__((aligned(64), packed)) ultimate_type_t;

/* Additional GCC-specific attributes */
struct __attribute__((packed)) packed_struct {
    scalar_char a;
    scalar_int b;
    scalar_double c;
};

union __attribute__((aligned(16))) aligned_union {
    scalar_long long_val;
    scalar_double double_val;
    struct packed_struct packed;
};

/* Forward declaration for circular reference */
struct circular_ref;

struct circular_ref {
    scalar_int value;
    struct circular_ref *next;
    struct circular_ref *prev;
    void (*update)(struct circular_ref*);
};

/* Type containing all categories in one */
typedef struct type_kitchen_sink {
    /* TYPE_SCALAR */
    scalar_int counter;
    
    /* TYPE_STRING */
    const char *description;
    
    /* TYPE_STRUCT (nested) */
    struct {
        scalar_float x, y, z;
    } coordinates;
    
    /* TYPE_USER_STRUCT */
    user_struct_t metadata;
    
    /* TYPE_UNION */
    union {
        scalar_int as_int;
        scalar_double as_double;
        void *as_ptr;
    } value;
    
    /* TYPE_POINTER */
    struct type_kitchen_sink *next;
    
    /* TYPE_ARRAY */
    callback_int_int callbacks[5];
    
    /* TYPE_CALLBACK */
    ultimate_type_t *(*factory)(scalar_int, const char*);
    
    /* TYPE_LANG_STRUCT pointer */
    struct lang_decl *gcc_internal;
    
    /* TYPE_UNDEFINED pointer */
    struct undefined_struct *unknown;
} kitchen_sink_t;

#endif /* TEST_TYPES_H */
