#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations creating undefined types initially */
struct opaque;              /* Will be TYPE_UNDEFINED */
union hidden_union;         /* Will be TYPE_UNDEFINED */
typedef struct forward_declared forward_t;

/* ==================== TYPE_SCALAR ==================== */
/* Basic scalar types and typedefs */
typedef int scalar_int;
typedef char scalar_char;
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;

/* GCC extensions for scalar types */
typedef __complex__ double complex_scalar;
typedef int __attribute__((vector_size(16))) vector_scalar;
typedef __builtin_va_list va_list_scalar;

/* ==================== TYPE_STRING ==================== */
typedef const char* string_type;
typedef char* mutable_string;

/* ==================== TYPE_STRUCT ==================== */
/* Regular struct */
struct regular_struct {
    int x;
    double y;
    char z;
};

/* Packed struct - likely TYPE_USER_STRUCT */
struct __attribute__((packed)) packed_struct {
    int a;
    double b;
    char c;
} __attribute__((aligned(16)));

/* Struct with designated initializer attribute */
struct __attribute__((designated_init)) designated_struct {
    int field1;
    double field2;
};

/* ==================== TYPE_UNION ==================== */
union basic_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Tagged union */
union __attribute__((aligned(8))) aligned_union {
    long long big;
    char small[8];
};

/* ==================== TYPE_POINTER ==================== */
/* Various pointer types */
typedef int* int_ptr;
typedef struct regular_struct* struct_ptr;
typedef void (*void_func_ptr)(void);
typedef const volatile char* complex_ptr;

/* ==================== TYPE_ARRAY ==================== */
/* Fixed-size arrays */
typedef int fixed_array[10];
typedef struct regular_struct struct_array[5];

/* Multi-dimensional array */
typedef int matrix[3][3];

/* Incomplete array type (in struct) */
struct with_incomplete_array {
    int count;
    int data[];  /* TYPE_ARRAY classification */
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types */
typedef int (*binary_op)(int, int);
typedef void (*event_callback)(void* user_data, int event_type);
typedef struct regular_struct* (*struct_factory)(int id);

/* Callback with complex signature */
typedef int (*complex_callback)(struct regular_struct*, union basic_union, ...);

/* ==================== RECURSIVE & INTERCONNECTED TYPES ==================== */
/* Self-referential struct */
struct recursive_node {
    int value;
    struct recursive_node* next;  /* TYPE_POINTER to same struct */
    struct recursive_node* prev;
};

/* Mutually recursive types */
struct type_a;
struct type_b;

struct type_a {
    int id;
    struct type_b* partner;  /* Forward reference */
};

struct type_b {
    int id;
    struct type_a* partner;
    struct type_a array_of_a[2];
};

/* Union containing array of pointers */
union container_union {
    struct type_a* a_ptrs[4];
    struct type_b* b_ptrs[2];
    binary_op callbacks[3];
};

/* ==================== COMPLEX NESTED TYPE ==================== */
struct master_container {
    struct recursive_node node;
    union container_union u;
    fixed_array numbers;
    matrix transform;
    event_callback on_event;
    struct with_incomplete_array* flexible;
};

/* ==================== OPAQUE TYPE DEFINITION ==================== */
/* Now define previously forward-declared opaque struct */
struct opaque {
    void* internal_data;
    int magic_number;
};

/* Define the forward-declared union */
union hidden_union {
    struct opaque* ptr;
    long long value;
};

/* Define forward-declared struct */
struct forward_declared {
    forward_t* next;  /* Recursive typedef */
    char name[32];
};

/* ==================== LANGUAGE-SPECIFIC STRUCTS ==================== */
/* Use C++ mode to potentially create TYPE_LANG_STRUCT */
#ifdef __cplusplus
extern "C++" {
    class cpp_class {
    private:
        int private_data;
    public:
        virtual ~cpp_class() {}
        virtual void method() = 0;
    };
    
    struct __attribute__((transaction_safe)) transaction_struct {
        int consistent_value;
        double atomic_double;
    };
}
#else
/* For C, use transaction_safe attribute directly */
struct __attribute__((transaction_safe)) transaction_struct {
    int consistent_value;
    double atomic_double;
};
#endif

/* Struct with GCC-specific attributes combination */
struct __attribute__((aligned(32), packed, cold)) attributed_struct {
    unsigned char flags;
    unsigned int bits : 4;
    unsigned int more_bits : 12;
    long double extended;
};

/* ==================== FUNCTION PROTOTYPES ==================== */
/* Functions using the complex types */
void process_struct(struct regular_struct* s);
struct master_container* create_container(int size);
int execute_callback(binary_op op, int a, int b);

#endif /* VARIED_TYPES_H */
