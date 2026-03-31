#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct undefined_struct;
union undefined_union;
typedef struct undefined_typedef_struct undefined_typedef_t;

/* TYPE_SCALAR: Fundamental scalar types */
int global_int;
float global_float;
double global_double;
char global_char;
long global_long;
short global_short;
unsigned int global_uint;
_Bool global_bool;

/* TYPE_STRING: String types */
const char* global_string_ptr = "global string";
char global_string_array[] = "array string";
const char* const global_const_string_ptr = "constant string";

/* TYPE_STRUCT: Complete struct types */
struct simple_struct {
    int x;
    float y;
    char z;
};

struct complex_struct {
    int id;
    double values[10];
    struct simple_struct* nested;
    char name[50];
};

struct packed_struct {
    char a;
    int b;
    double c;
} __attribute__((packed));

/* TYPE_USER_STRUCT: Typedef struct types */
typedef struct {
    int data;
    float precision;
} user_struct_t;

typedef struct node {
    int value;
    struct node* next;
} linked_list_t;

/* TYPE_UNION: Union types */
union simple_union {
    int as_int;
    float as_float;
    char as_char;
};

union complex_union {
    struct simple_struct as_struct;
    user_struct_t as_user;
    void* as_pointer;
} __attribute__((aligned(16)));

/* TYPE_POINTER: Various pointer types */
int* int_ptr;
struct simple_struct* struct_ptr;
union simple_union* union_ptr;
user_struct_t* user_struct_ptr;
void* void_ptr;
const volatile int* cv_int_ptr;

/* TYPE_ARRAY: Arrays of different types */
int int_array[100];
float float_array[10][20];
struct simple_struct struct_array[5];
user_struct_t user_struct_array[3][4];
int* pointer_array[50];
char* string_array[] = {"one", "two", "three"};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*simple_callback_t)(int, int);
typedef void (*complex_callback_t)(struct complex_struct*, user_struct_t*);
typedef char* (*string_callback_t)(const char*, int);
typedef void (*void_callback_t)(void);

/* Nested callback in struct */
struct callback_container {
    simple_callback_t simple_cb;
    complex_callback_t complex_cb;
    void_callback_t void_cb;
};

/* Complex nested type example */
typedef struct nested_example {
    int id;
    union complex_union data;
    struct nested_example* children[10];
    simple_callback_t processor;
    char* (*name_generator)(struct nested_example*);
} nested_example_t;

/* TYPE_LANG_STRUCT: GCC internal structure (tree_node from GCC's tree representation) */
struct tree_node;
struct tree_common {
    struct tree_node* chain;
    int code;
};

/* Transparent union for GCC attributes */
typedef union __attribute__((transparent_union)) {
    int* int_ptr;
    void* void_ptr;
} transparent_union_t;

/* More complex nesting */
typedef struct container {
    /* Array of pointers to unions */
    union simple_union* union_ptrs[20];
    
    /* Function pointer returning pointer to struct with callback */
    struct callback_container* (*get_callbacks)(int);
    
    /* Nested anonymous struct */
    struct {
        int depth;
        nested_example_t** items;
    } hierarchy;
    
    /* Flexible array member */
    int flexible_array[];
} container_t;

/* Even more complex: struct containing array of pointers to unions with callbacks */
struct ultimate_nest {
    container_t* containers[5];
    transparent_union_t transparent_data;
    struct tree_common* tree_data;  /* Potential lang struct reference */
    
    /* Callback that takes array and returns pointer */
    nested_example_t* (*process_array)(struct ultimate_nest*, int[][10]);
};

/* Function pointer with complex return type */
typedef struct ultimate_nest* (*factory_fn_t)(
    int count,
    const char* name,
    simple_callback_t validator
);

/* Mixed declarations with attributes */
struct __attribute__((aligned(64))) aligned_struct {
    long double big_data[8];
    volatile int counter;
};

union __attribute__((packed)) packed_union {
    struct aligned_struct as_aligned;
    struct packed_struct as_packed;
};

#endif /* TEST_TYPES_H */
