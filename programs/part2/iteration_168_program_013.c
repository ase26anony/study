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
typedef unsigned int scalar_uint;
typedef _Bool scalar_bool;

/* TYPE_STRING: String types and literals */
#define STRING_LITERAL "test_string_literal"
extern const char *string_pointer;
typedef const char *string_ptr_t;

/* TYPE_STRUCT: Complete struct types */
struct simple_struct {
    int id;
    float value;
    char name[32];
};

struct complex_struct {
    struct simple_struct base;
    double *data_ptr;
    int array[10];
    struct complex_struct *next;
};

struct packed_struct {
    char a;
    int b;
    double c;
} __attribute__((packed));

/* TYPE_USER_STRUCT: Typedef'd struct types */
typedef struct {
    int x;
    int y;
    int z;
} user_struct_t;

typedef struct nested_user_struct {
    user_struct_t point;
    struct nested_user_struct *parent;
    void *user_data;
} nested_user_t;

/* TYPE_UNION: Union types */
union simple_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

union complex_union {
    struct simple_struct as_struct;
    user_struct_t as_user;
    double as_array[4];
    union simple_union nested;
} __attribute__((aligned(16)));

/* Transparent union for GCC attribute */
typedef union {
    int *int_ptr;
    float *float_ptr;
    char *char_ptr;
} transparent_union_t __attribute__((transparent_union));

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr_t;
typedef struct simple_struct *struct_ptr_t;
typedef union simple_union *union_ptr_t;
typedef void (*func_ptr_t)(void);
typedef const volatile void *cv_void_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int scalar_array_1d[10];
typedef float scalar_array_2d[5][5];
typedef struct simple_struct struct_array[20];
typedef int_ptr_t pointer_array[15];
typedef int (*func_ptr_array[8])(void);

/* Multi-dimensional complex array */
typedef union complex_union complex_array_3d[3][3][3];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*simple_callback)(void);
typedef void (*complex_callback)(int, float, const char*);
typedef struct simple_struct *(*struct_returning_callback)(int param);
typedef int (*nested_callback)(int (*inner)(float), void *context);

/* Callback with array parameter */
typedef void (*array_param_callback)(int arr[], size_t len);

/* Callback returning callback */
typedef simple_callback (*callback_returning_callback)(int mode);

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */
/* These are typically from GCC's internal representations */
struct tree_common;
struct tree_type;
struct tree_decl;

/* Dummy structures that might match GCC's internal patterns */
struct lang_type {
    unsigned int lang_flag_1 : 1;
    unsigned int lang_flag_2 : 1;
    void *lang_specific;
};

struct lang_decl {
    struct tree_decl *chain;
    int decl_flags;
};

/* Complex nested type combining multiple categories */
typedef struct container_struct {
    /* Scalar members */
    int id;
    float priority;
    
    /* String member */
    const char *name;
    
    /* Struct member */
    user_struct_t position;
    
    /* Union member */
    union simple_union data;
    
    /* Pointer members */
    struct container_struct *next;
    void **generic_ptrs;
    
    /* Array members */
    int counts[10];
    struct simple_struct items[5];
    
    /* Callback member */
    complex_callback notify;
    
    /* Nested pointer to array of callbacks */
    simple_callback (*handler_table)[4];
    
    /* Language structure pointer */
    struct lang_type *lang_info;
    
    /* Attribute for alignment */
    unsigned char padding[16];
} __attribute__((aligned(32))) container_t;

/* Function pointer type with complex signature */
typedef container_t *(*factory_callback)(
    int type,
    const char *name,
    int (*validator)(container_t*),
    void *(*allocator)(size_t)
);

/* Union containing function pointers */
union callback_union {
    simple_callback simple;
    complex_callback complex;
    factory_callback factory;
    void (*generic)(void);
};

/* Array of unions containing various callbacks */
typedef union callback_union callback_array[10];

/* Final master structure containing all type categories */
typedef struct type_master {
    /* Undefined type pointer */
    struct undefined_struct *undefined_ptr;
    
    /* Scalars */
    scalar_int master_id;
    scalar_double master_value;
    
    /* String */
    const char *master_name;
    
    /* Structs */
    struct complex_struct nested_struct;
    user_struct_t user_struct;
    
    /* Union */
    union complex_union data_union;
    
    /* Pointers */
    int *int_ptr_array[5];
    struct type_master **self_ptr_ptr;
    
    /* Arrays */
    scalar_array_2d matrix;
    struct_array object_array;
    
    /* Callbacks */
    nested_callback recursive_handler;
    callback_returning_callback callback_factory;
    
    /* Language structure */
    struct lang_type lang_type_instance;
    
    /* Nested container */
    container_t subcontainer;
    
    /* Complex nested type: pointer to array of function pointers returning struct pointers */
    struct simple_struct *(*(*complex_nested)[3])(int);
} type_master_t;

/* Global instances to ensure types are used */
extern type_master_t global_master;
extern callback_array global_callbacks;
extern complex_array_3d global_3d_array;

#endif /* TEST_TYPES_H */
