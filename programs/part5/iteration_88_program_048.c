#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* TYPE_UNDEFINED initially */
struct forward_declared_struct;    /* Another undefined type */
typedef struct incomplete *incomplete_ptr_t; /* Pointer to undefined */

/* ========== TYPE_SCALAR / Basic Types ========== */
typedef int my_int;                /* Simple scalar typedef */
typedef char my_char;
typedef _Bool my_bool;
typedef __complex__ double complex_double;  /* GNU extension */
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) vector_int; /* SIMD vector */

/* ========== TYPE_STRING ========== */
typedef const char* string_ptr;
typedef char* mutable_string;

/* ========== TYPE_ARRAY ========== */
typedef int int_array_10[10];
typedef int int_array_unknown[];  /* Incomplete array type */
typedef int (*array_of_ptrs)[5];  /* Pointer to array */

/* ========== TYPE_POINTER ========== */
typedef int* int_ptr;
typedef void* generic_ptr;
typedef const volatile int* cv_int_ptr;
typedef int** ptr_to_ptr;

/* ========== TYPE_CALLBACK / Function Pointers ========== */
typedef int (*binary_op)(int, int);
typedef void (*callback_t)(void* data, int value);
typedef int (*va_func)(int, ...);
typedef __builtin_va_list va_list_alias;

/* ========== TYPE_STRUCT ========== */
struct simple_struct {
    int x;
    double y;
    char z;
};

/* TYPE_USER_STRUCT with attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(16)));

struct __attribute__((designated_init)) designated_init_struct {
    int field1;
    double field2;
};

/* Struct with incomplete array as last member (flexible array member) */
struct flex_array_struct {
    int count;
    int data[];  /* TYPE_ARRAY - incomplete */
};

/* ========== TYPE_UNION ========== */
union simple_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

union __attribute__((packed)) packed_union {
    char bytes[8];
    long long value;
};

/* ========== Complex Recursive/Interconnected Types ========== */

/* Recursive struct with pointer to self */
struct recursive_node {
    int value;
    struct recursive_node* next;  /* TYPE_POINTER to same struct */
    struct recursive_node* prev;
};

/* Mutual recursion between two structs */
struct type_a;
struct type_b;

struct type_a {
    int id;
    struct type_b* partner;  /* Forward reference */
};

struct type_b {
    int id;
    struct type_a* partner;
    struct type_a array_of_a[3];  /* TYPE_ARRAY of struct */
};

/* Union containing array of pointers */
union container_union {
    struct type_a* a_ptrs[5];     /* Array of pointers */
    struct type_b* b_ptr;
    binary_op func_ptr;           /* Callback type */
};

/* ========== Opaque struct definition (was forward declared) ========== */
struct opaque {
    void* secret_data;
    int hidden_value;
    struct opaque* next;  /* Pointer to same opaque type */
};

/* ========== Struct with all type varieties ========== */
struct kitchen_sink {
    /* Scalars */
    my_int custom_int;
    complex_double cplx;
    vector_int simd;
    
    /* Strings */
    string_ptr name;
    mutable_string buffer;
    
    /* Arrays */
    int_array_10 fixed_array;
    int* dynamic_array;  /* Will be initialized with malloc */
    
    /* Pointers */
    int_ptr int_pointer;
    generic_ptr void_pointer;
    ptr_to_ptr double_pointer;
    
    /* Structs and Unions */
    struct simple_struct embedded;
    union simple_union choice;
    
    /* Callback */
    callback_t notify;
    
    /* Recursive reference */
    struct kitchen_sink* self;
    
    /* Opaque pointer */
    struct opaque* opaque_data;
    
    /* Array of function pointers */
    binary_op operations[3];
};

/* ========== Extern "C++" block for TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
extern "C++" {
    class CppClass {
    private:
        int private_data;
    public:
        virtual ~CppClass() {}
        virtual void method() = 0;
        static int static_method(int x) { return x * 2; }
    };
    
    struct CppStruct {
        int x;
        double y;
        CppClass* obj;
    };
}
#endif

/* ========== Transaction-safe struct (GCC extension) ========== */
struct __attribute__((transaction_safe)) transaction_struct {
    int account_id;
    double balance;
    void (*commit)(struct transaction_struct*);  /* Callback pointer */
};

/* ========== Function declarations using the types ========== */
void process_struct(struct simple_struct* s);
int calculate(binary_op op, int a, int b);
struct recursive_node* create_node(int value);
void traverse_list(struct recursive_node* head);

/* ========== Global variables for multi-file testing ========== */
extern struct kitchen_sink global_sink;
extern union container_union global_container;

#endif /* VARIED_TYPES_H */
