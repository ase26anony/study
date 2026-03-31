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
typedef _Bool scalar_bool;
typedef long scalar_long;
typedef long long scalar_longlong;

/* TYPE_STRING: String types */
typedef const char* string_ptr;
typedef char string_array[32];

/* TYPE_STRUCT: Complete struct types */
struct simple_struct {
    scalar_int id;
    scalar_float value;
    char name[16];
};

struct complex_struct {
    struct simple_struct base;
    scalar_double *data_ptr;
    int data_array[10];
    struct complex_struct *next;
} __attribute__((packed));

/* TYPE_USER_STRUCT: Typedef'd struct types */
typedef struct {
    scalar_int x;
    scalar_float y;
    char label[20];
} user_struct_t;

typedef struct nested_user_struct {
    user_struct_t user;
    struct nested_user_struct *parent;
    scalar_double matrix[3][3];
} nested_user_t;

/* TYPE_UNION: Union types */
union simple_union {
    scalar_int as_int;
    scalar_float as_float;
    char as_char[4];
};

union complex_union {
    struct simple_struct as_struct;
    user_struct_t as_user;
    scalar_double as_double;
    void *as_pointer;
} __attribute__((aligned(16)));

/* TYPE_POINTER: Various pointer types */
typedef scalar_int *scalar_ptr;
typedef struct simple_struct *struct_ptr;
typedef user_struct_t *user_struct_ptr;
typedef union simple_union *union_ptr;
typedef void (*func_ptr)(void);
typedef const volatile char *cv_ptr;

/* TYPE_ARRAY: Arrays of different types */
typedef scalar_int scalar_array[100];
typedef struct simple_struct struct_array[50];
typedef user_struct_t *pointer_array[20];
typedef scalar_int multi_dim_array[5][10][15];
typedef const char *string_array_t[10];

/* TYPE_CALLBACK: Function pointer types */
typedef scalar_int (*int_callback)(scalar_int, scalar_float);
typedef void (*void_callback)(struct simple_struct*, user_struct_t*);
typedef user_struct_t* (*struct_callback)(scalar_int, const char*);
typedef scalar_double (*complex_callback)(scalar_int*, struct simple_struct**, 
                                         void (*nested_callback)(void));

/* TYPE_LANG_STRUCT: GCC internal structure (tree_node) */
struct tree_node;
typedef struct tree_node *tree_ptr;
struct tree_common {
    tree_ptr chain;
    tree_ptr type;
    int uid;
};

/* Complex nested type combining multiple categories */
typedef struct container_struct {
    /* Mix of different type categories */
    scalar_int id;                     /* TYPE_SCALAR */
    char name[32];                     /* TYPE_STRING */
    struct simple_struct base;         /* TYPE_STRUCT */
    user_struct_t user_data;           /* TYPE_USER_STRUCT */
    union simple_union data_union;     /* TYPE_UNION */
    
    /* Pointers to various types */
    void *generic_ptr;                 /* TYPE_POINTER */
    struct container_struct *self_ptr; /* TYPE_POINTER */
    int_callback callback;             /* TYPE_CALLBACK */
    
    /* Arrays */
    scalar_int scores[10];             /* TYPE_ARRAY */
    user_struct_t *users[5];           /* TYPE_ARRAY of pointers */
    
    /* Nested function pointer with complex signature */
    struct container_struct* (*factory)(int, const char*, 
                                       void (*init)(struct container_struct*));
    
    /* Pointer to undefined type */
    struct undefined_struct *undefined; /* TYPE_UNDEFINED */
    
    /* GCC attributes */
    int packed_data[4] __attribute__((packed));
    char aligned_buffer[64] __attribute__((aligned(32)));
} container_t;

/* Another complex type: function returning pointer to struct with callback */
typedef container_t* (*container_factory_t)(
    int count,
    const char *name,
    void (*configure)(container_t*, user_struct_t*),
    union complex_union *data
);

/* Transparent union for GCC attribute */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    float *float_ptr;
    void *void_ptr;
} transparent_union_t;

/* Even more complex nesting */
typedef struct ultimate_nest {
    container_t main;
    container_factory_t factory;
    struct {
        int depth;
        struct ultimate_nest *deeper;
    } nested_anon;
    
    /* Array of function pointers */
    int_callback callbacks[8];
    
    /* Pointer to array of structs */
    user_struct_t (*user_matrix)[10][10];
    
    /* Callback returning pointer to array */
    int (*get_matrix)[5][5](void);
    
    /* Reference to language struct */
    tree_ptr ast_node;
} ultimate_nest_t;

#endif /* TEST_TYPES_H */
