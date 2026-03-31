#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct *undefined_ptr_t;

/* TYPE_SCALAR: Fundamental types */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef char scalar_char;
typedef long scalar_long;
typedef short scalar_short;
typedef unsigned int scalar_uint;
typedef _Bool scalar_bool;

/* TYPE_STRING: String types */
typedef const char* string_ptr;
typedef char string_array[32];

/* TYPE_STRUCT: Complete struct types */
struct simple_struct {
    int id;
    float value;
    char name[32];
};

struct complex_struct {
    scalar_int counter;
    scalar_double precision;
    string_ptr description;
    struct simple_struct nested;
    char buffer[256];
};

/* TYPE_USER_STRUCT: Typedef'd struct types */
typedef struct {
    int x;
    int y;
    int z;
} user_struct_t;

typedef struct tagged_struct {
    user_struct_t coordinates;
    scalar_float weight;
    scalar_char label;
} tagged_struct_t;

/* TYPE_UNION: Union types */
union simple_union {
    int as_int;
    float as_float;
    char as_char;
    void* as_ptr;
};

union complex_union {
    struct simple_struct as_struct;
    user_struct_t as_user;
    scalar_double as_double;
    long as_long_array[4];
};

/* Transparent union (GCC-specific attribute) */
typedef union __attribute__((transparent_union)) transparent_union_t {
    int* int_ptr;
    void* void_ptr;
    const char* str_ptr;
} transparent_union_t;

/* TYPE_POINTER: Various pointer types */
typedef struct simple_struct* struct_ptr;
typedef union simple_union* union_ptr;
typedef user_struct_t* user_struct_ptr;
typedef int (*func_ptr)(void);
typedef void (*void_func_ptr)(int, char*);
typedef const volatile int* cv_ptr;

/* TYPE_ARRAY: Arrays of different types */
typedef int scalar_array[10];
typedef struct simple_struct struct_array[5];
typedef user_struct_t user_struct_array[8];
typedef int* pointer_array[20];
typedef int multi_dim_array[3][4][5];
typedef char string_array_2d[5][64];

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef int (*callback_int_int)(int);
typedef float (*callback_float_struct)(struct simple_struct*);
typedef void (*callback_void_multi)(int, float, char*, const void*);
typedef user_struct_t* (*callback_user_ptr)(int, union complex_union);
typedef int (*callback_nested)(int (*)(float), void*);

/* Complex nested callback */
typedef void (*complex_callback)(
    struct complex_struct*,
    callback_int_int,
    user_struct_array
);

/* TYPE_LANG_STRUCT: GCC internal structure (dummy declaration) */
struct tree_node;
struct tree_common;
struct tree_type;

/* Packed struct with GCC attribute */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
    double d;
};

/* Aligned struct with GCC attribute */
struct __attribute__((aligned(32))) aligned_struct {
    int data[8];
    long long counter;
};

/* Complex nested type hierarchy */
typedef struct container {
    /* Mixed member types covering all categories */
    scalar_int count;                     /* TYPE_SCALAR */
    string_ptr name;                      /* TYPE_STRING */
    struct simple_struct base;            /* TYPE_STRUCT */
    user_struct_t user;                   /* TYPE_USER_STRUCT */
    union simple_union data_union;        /* TYPE_UNION */
    struct container* next;               /* TYPE_POINTER */
    callback_int_int processor;           /* TYPE_CALLBACK */
    user_struct_t items[16];              /* TYPE_ARRAY */
    struct tree_node* tree_node;          /* TYPE_LANG_STRUCT */
    
    /* Nested arrays of pointers */
    void* ptr_array[8];
    
    /* Array of function pointers */
    callback_void_multi handlers[4];
    
    /* Union containing struct with callback */
    union {
        struct {
            int type;
            complex_callback handler;
        } callback_data;
        struct {
            float x, y;
            int (*transform)(float, float);
        } coord_data;
    } variant;
} container_t;

/* Function pointer returning pointer to struct with callback members */
typedef container_t* (*factory_func)(
    int size,
    const char* name,
    callback_int_int init_func
);

/* Even more complex nesting */
typedef struct mega_nested {
    /* Array of pointers to unions containing structs with callbacks */
    union complex_union* union_ptrs[10];
    
    /* Pointer to array of function pointers */
    complex_callback (*callback_matrix)[5];
    
    /* Struct containing union containing array of struct pointers */
    struct {
        int tag;
        union {
            struct simple_struct* structs[8];
            user_struct_t* users[8];
            void* generic[16];
        } data;
    } collection;
    
    /* Self-referential pointer */
    struct mega_nested* self;
    
    /* Pointer to callback that returns pointer to struct */
    factory_func create;
} mega_nested_t;

/* Void pointer usage for TYPE_UNDEFINED */
extern void* external_undefined;

/* Function declarations using all types */
void process_scalar(scalar_int a, scalar_float b, scalar_double c);
string_ptr create_string(const char* input);
struct simple_struct* create_struct(int id, float value, const char* name);
user_struct_t transform_user_struct(user_struct_t input, callback_int_int func);
union simple_union create_union_from_int(int value);
int process_array(scalar_array arr, int size);
void execute_callback(callback_void_multi cb, int arg1, float arg2, char* arg3);
container_t* create_container(const char* name, int capacity);
mega_nested_t* create_mega_nested(factory_func factory);

#endif /* TEST_TYPES_H */
