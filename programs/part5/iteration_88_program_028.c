#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* TYPE_UNDEFINED initially */
struct forward_declared_struct;    /* Another undefined type */
typedef struct incomplete incomplete_t;

/* ========== TYPE_SCALAR / Basic Types ========== */
typedef int scalar_int;            /* Simple scalar typedef */
typedef unsigned long scalar_ulong;
typedef _Bool scalar_bool;
typedef char scalar_char;

/* GNU extensions for scalar types */
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) vector_int;
typedef float __attribute__((vector_size(32))) vector_float;

/* Builtin types */
typedef __builtin_va_list va_list_type;

/* ========== TYPE_STRING ========== */
typedef const char* string_ptr;
typedef char* mutable_string;

/* ========== TYPE_STRUCT / Regular Structs ========== */
struct regular_struct {
    int field1;
    double field2;
    char field3;
};

/* Packed struct for TYPE_USER_STRUCT */
struct __attribute__((packed)) packed_user_struct {
    int x;
    double y;
    char z;
} __attribute__((aligned(16)));

/* Struct with designated init attribute */
struct __attribute__((designated_init)) designated_struct {
    int a;
    float b;
    char c;
};

/* ========== TYPE_UNION ========== */
union basic_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Tagged union */
union __attribute__((aligned(8))) tagged_union {
    struct {
        int type;
        char data[16];
    } header;
    struct {
        int x, y, z;
    } coords;
};

/* ========== TYPE_POINTER ========== */
typedef struct regular_struct* regular_struct_ptr;
typedef union basic_union* union_ptr;
typedef void (*generic_func_ptr)(void);
typedef int* int_ptr;
typedef int_ptr* pointer_to_pointer;

/* ========== TYPE_ARRAY ========== */
typedef int fixed_array[10];
typedef int multi_dim_array[5][5];
extern int incomplete_array[];  /* Incomplete array type */

/* Struct with flexible array member */
struct flex_array_struct {
    int count;
    int data[];  /* Incomplete array as last member */
};

/* ========== TYPE_CALLBACK / Function Pointers ========== */
typedef int (*binary_op)(int, int);
typedef void (*event_callback)(void* user_data, int event_type);
typedef int (*variadic_func)(int count, ...);

/* Complex callback with struct parameter */
typedef struct regular_struct* (*struct_factory)(int id);
typedef void (*struct_processor)(struct regular_struct*);

/* ========== Recursive and Interconnected Types ========== */
/* Self-referential struct */
struct recursive_node {
    int value;
    struct recursive_node* next;  /* Pointer to own type */
    struct recursive_node* prev;
};

/* Mutual recursion between two structs */
struct type_a;
struct type_b;

struct type_a {
    int id;
    struct type_b* partner;
};

struct type_b {
    int id;
    struct type_a* partner;
    struct type_a* alternatives[3];  /* Array of pointers */
};

/* Union containing array of pointers */
union container_union {
    struct type_a* a_ptrs[5];
    struct type_b* b_ptrs[3];
    binary_op func_ptrs[4];
};

/* ========== Opaque Struct Definition (was TYPE_UNDEFINED) ========== */
struct opaque {
    void* internal_data;
    int magic_number;
    struct opaque* next;  /* Linked list of opaque structs */
};

/* ========== Complex Nested Type ========== */
struct nested_mess {
    union {
        struct {
            int x;
            fixed_array arr;
        } part1;
        struct {
            float y;
            struct regular_struct* rs_ptr;
            event_callback callback;
        } part2;
    } data;
    
    struct nested_mess* children[4];
    binary_op operations[2];
};

/* ========== C++ Specific for TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
extern "C++" {
    /* This should generate TYPE_LANG_STRUCT in C++ mode */
    struct cpp_specific_struct {
        int cpp_field;
        void cpp_method();
    } __attribute__((transaction_safe));
    
    /* Class with virtual methods */
    class BaseClass {
    public:
        virtual void vfunc() = 0;
        virtual ~BaseClass() {}
    };
    
    class DerivedClass : public BaseClass {
    public:
        virtual void vfunc() override;
        int derived_field;
    };
}
#endif

/* ========== Transaction-safe attribute ========== */
struct __attribute__((transaction_safe)) transaction_struct {
    int atomic_counter;
    void* protected_data;
};

/* ========== Function Declarations ========== */
void process_types(void);
struct recursive_node* create_recursive_list(int count);
int sum_array(const int* arr, int size);

#endif /* VARIED_TYPES_H */
