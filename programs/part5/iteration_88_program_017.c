#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct undefined_struct;
struct opaque;

/* TYPE_SCALAR: Various scalar types and typedefs */
typedef char byte_t;
typedef int integer_t;
typedef long long int64_t;
typedef float float32_t;
typedef double float64_t;
typedef _Bool bool_t;

/* GNU extensions for scalar types */
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) int_vector;
typedef float __attribute__((vector_size(32))) float_vector;

/* Builtin types */
typedef __builtin_va_list va_list_t;

/* TYPE_STRING: String type */
typedef const char* string_t;

/* TYPE_STRUCT: Regular struct definitions */
struct point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(8)));

struct __attribute__((designated_init)) designated_struct {
    int field1;
    float field2;
    char field3;
};

/* TYPE_UNION: Union definitions */
union data_union {
    int as_int;
    float as_float;
    char as_bytes[4];
    void* as_pointer;
};

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr_t;
typedef struct point* point_ptr_t;
typedef void (*generic_func_ptr_t)(void);
typedef const volatile char* special_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef float matrix_3x3[3][3];
extern int incomplete_array[];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*event_handler_t)(int event_id, void* user_data);
typedef char* (*string_formatter_t)(const char* fmt, ...);

/* Recursive and mutually recursive types */
struct recursive_node {
    int data;
    struct recursive_node* next;  /* TYPE_POINTER to same struct */
};

struct type_a;
struct type_b;

struct type_a {
    int id;
    struct type_b* partner;  /* Forward reference */
};

struct type_b {
    int id;
    struct type_a* partner;  /* Mutual recursion */
    struct recursive_node* node_list;
};

/* Incomplete array as last member (flexible array member) */
struct flexible_array {
    int count;
    double data[];  /* TYPE_ARRAY (incomplete) */
};

/* Now define the previously opaque struct */
struct opaque {
    int secret;
    struct undefined_struct* link;  /* Still undefined */
};

/* C++ specific section for TYPE_LANG_STRUCT */
#ifdef __cplusplus
extern "C++" {
    class cpp_class {
    private:
        int private_data;
    public:
        cpp_class() : private_data(0) {}
        virtual ~cpp_class() {}
        virtual void method() = 0;
    };
    
    struct __attribute__((transaction_safe)) transaction_struct {
        int value;
        void update(int new_val) __attribute__((transaction_safe));
    };
}
#endif

/* Complex nested type combining multiple classifications */
struct master_container {
    /* TYPE_STRUCT */
    struct point position;
    
    /* TYPE_UNION */
    union data_union storage;
    
    /* TYPE_POINTER */
    struct recursive_node* node_ptr;
    
    /* TYPE_ARRAY */
    int id_list[5];
    
    /* TYPE_CALLBACK */
    comparator_t compare_func;
    
    /* TYPE_STRING */
    const char* name;
    
    /* TYPE_USER_STRUCT (due to attribute) */
    struct __attribute__((packed)) packed_struct packed_data;
    
    /* Incomplete type pointer */
    struct undefined_struct* future;
};

/* Global callback function type */
typedef void (*global_callback_t)(struct master_container*, union data_union);

#endif /* VARIED_TYPES_H */
