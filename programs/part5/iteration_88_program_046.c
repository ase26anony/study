#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations that will be TYPE_UNDEFINED initially */
struct opaque;
struct forward_declared;
union forward_union;

/* ==================== TYPE_SCALAR ==================== */
/* Basic scalar typedefs */
typedef char byte_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef float float32_t;
typedef double float64_t;
typedef _Bool bool_t;
typedef enum { RED, GREEN, BLUE } color_t;

/* GCC extensions for scalar types */
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) int_vec4;
typedef float __attribute__((vector_size(32))) float_vec8;

/* ==================== TYPE_STRING ==================== */
typedef const char* string_t;
typedef char* mutable_string_t;

/* ==================== TYPE_USER_STRUCT ==================== */
/* Structs with attributes that make them TYPE_USER_STRUCT */
struct __attribute__((packed, aligned(2))) packed_struct {
    char a;
    int b;
    short c;
};

struct __attribute__((aligned(64))) overaligned_struct {
    double data[8];
};

struct __attribute__((designated_init)) designated_init_struct {
    int field1;
    char field2;
    float field3;
};

/* ==================== TYPE_STRUCT ==================== */
/* Regular struct definitions */
struct point {
    int x;
    int y;
    int z;
};

struct node {
    int id;
    char name[32];
    struct node* next;
    struct node* prev;
};

/* ==================== TYPE_UNION ==================== */
union data_container {
    int as_int;
    float as_float;
    double as_double;
    char as_bytes[8];
    void* as_pointer;
};

/* Tagged union */
struct tagged_union {
    int type;
    union {
        int int_val;
        float float_val;
        char* string_val;
        void* ptr_val;
    } data;
};

/* ==================== TYPE_POINTER ==================== */
/* Various pointer types */
typedef int* int_ptr_t;
typedef const char* const_string_ptr_t;
typedef void (*generic_func_ptr_t)(void);
typedef struct node* node_ptr_t;
typedef union data_container* container_ptr_t;

/* Pointer to incomplete type */
typedef struct opaque* opaque_ptr_t;

/* ==================== TYPE_ARRAY ==================== */
/* Fixed-size arrays */
typedef int int_array_10[10];
typedef struct point point_array_5[5];
typedef char* string_array_8[8];

/* Multi-dimensional arrays */
int matrix_3x3[3][3];
float tensor_2x2x2[2][2][2];

/* Incomplete array type (flexible array member) */
struct flex_array {
    int length;
    int data[];  /* TYPE_ARRAY with incomplete type */
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*event_handler_t)(int event_id, void* user_data);
typedef char* (*string_formatter_t)(const char* fmt, ...);
typedef int (*va_func_t)(int count, ...);

/* Complex callback with struct parameter */
typedef void (*node_visitor_t)(struct node* n, int depth, void* context);

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Structs that might become TYPE_LANG_STRUCT in C++ mode */
#ifdef __cplusplus
extern "C++" {
    struct cpp_specific {
        int cpp_field;
        virtual void method() = 0;
    };
}
#else
/* Use GCC attributes that might trigger language-specific handling */
struct __attribute__((transaction_safe)) transaction_safe_struct {
    int value;
    void (*update)(int);
};
#endif

/* ==================== COMPLEX NESTED TYPES ==================== */
/* Recursive struct with multiple pointer types */
struct recursive_node {
    int data;
    struct recursive_node* self_ptr;      /* Pointer to own type */
    struct recursive_node* children[4];   /* Array of pointers */
    void (*process)(struct recursive_node*);  /* Callback taking self */
};

/* Union containing array of pointers */
union pointer_union {
    struct node* node_ptrs[10];
    struct point* point_ptrs[5];
    void* generic_ptrs[20];
};

/* Struct containing all types */
struct type_kitchen_sink {
    /* Scalars */
    int scalar_int;
    float scalar_float;
    complex_double scalar_complex;
    
    /* Strings */
    string_t static_string;
    mutable_string_t dynamic_string;
    
    /* Structs */
    struct point point;
    struct packed_struct packed;
    
    /* Unions */
    union data_container container;
    
    /* Pointers */
    int_ptr_t int_ptr;
    node_ptr_t node_ptr;
    opaque_ptr_t opaque_ptr;
    
    /* Arrays */
    int_array_10 fixed_array;
    char variable_array[0];  /* Zero-length array */
    
    /* Callbacks */
    comparator_t compare_func;
    node_visitor_t visitor_func;
    
    /* Nested struct */
    struct {
        int nested_id;
        char nested_name[16];
    } nested;
};

/* ==================== OPAQUE TYPE DEFINITION ==================== */
/* Now define the previously opaque struct */
struct opaque {
    int magic;
    void* data;
    struct opaque* next;
};

/* ==================== FUNCTION DECLARATIONS ==================== */
/* Functions using the complex types */
void process_node(struct node* n, node_visitor_t visitor);
struct recursive_node* create_recursive_tree(int depth);
int compare_points(const void* a, const void* b);

/* Variadic function using builtin types */
int sum_variadic(int count, ...);

/* Function using GCC builtin types */
size_t process_va_list(__builtin_va_list args);

#endif /* VARIED_TYPES_H */
