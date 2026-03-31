#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* TYPE_UNDEFINED initially */
struct forward_declared_struct;    /* Another undefined type */
typedef struct incomplete incomplete_t;

/* ========== TYPE_SCALAR Definitions ========== */
typedef int scalar_int;
typedef char scalar_char;
typedef _Bool scalar_bool;
typedef __complex__ double complex_scalar;
typedef __complex__ float complex_float;
typedef int __attribute__((vector_size(16))) vector_int;
typedef float __attribute__((vector_size(32))) vector_float;

/* GNU extensions for scalar testing */
typedef __builtin_va_list va_list_scalar;
typedef __int128 int128_scalar;
typedef unsigned __int128 uint128_scalar;

/* ========== TYPE_STRING ========== */
typedef const char* string_ptr;
typedef char* mutable_string;

/* ========== TYPE_POINTER Definitions ========== */
typedef scalar_int* int_ptr;
typedef void* generic_ptr;
typedef struct opaque* opaque_ptr;
typedef incomplete_t* incomplete_ptr;

/* ========== TYPE_ARRAY Definitions ========== */
typedef int fixed_array[10];
typedef int incomplete_array[];
typedef int zero_length_array[0];
typedef int variable_len_array[__builtin_choose_expr(1, 5, 3)];

/* ========== TYPE_CALLBACK (Function Pointers) ========== */
typedef int (*simple_callback)(void);
typedef void (*complex_callback)(struct opaque*, int, ...);
typedef int (*filter_callback)(const char*, int);
typedef void (*recursive_callback)(struct forward_declared_struct*);

/* ========== TYPE_STRUCT Definitions ========== */
struct simple_struct {
    int x;
    double y;
    char z;
};

/* Packed struct for TYPE_USER_STRUCT */
struct __attribute__((packed, aligned(2))) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((designated_init));

/* Struct with attribute for TYPE_USER_STRUCT */
struct __attribute__((aligned(32))) aligned_struct {
    long long data[4];
    char metadata[16];
};

/* Struct with incomplete array at end */
struct flex_array_struct {
    int count;
    int data[];  /* TYPE_ARRAY classification */
};

/* Recursive struct with pointer to self */
struct recursive_struct {
    int value;
    struct recursive_struct* next;
    struct recursive_struct* prev;
};

/* Complex nested struct */
struct container {
    struct simple_struct simple;
    struct aligned_struct aligned;
    fixed_array arr;
    incomplete_ptr ptr;
};

/* ========== TYPE_UNION Definitions ========== */
union simple_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

union complex_union {
    struct simple_struct as_struct;
    struct aligned_struct as_aligned;
    struct recursive_struct* as_recursive_ptr;
    complex_callback as_callback;
    int as_array[8];
};

/* ========== TYPE_LANG_STRUCT via C++ extensions ========== */
#ifdef __cplusplus
extern "C++" {
    struct cpp_like_struct {
        int x;
        double y;
        
        /* Transaction-safe attribute may trigger TYPE_LANG_STRUCT */
        void method() __attribute__((transaction_safe));
    };
}
#else
/* Use GCC transaction attribute for C mode */
struct __attribute__((transaction_safe)) transaction_struct {
    int data;
    void (*update)(int);
};
#endif

/* ========== Complete previously undefined types ========== */
struct opaque {
    int magic;
    struct forward_declared_struct* link;
    incomplete_array flex;  /* Incomplete array member */
};

struct forward_declared_struct {
    struct opaque* backlink;
    recursive_callback callback;
    union complex_union data;
};

typedef struct incomplete {
    struct forward_declared_struct* ref;
    simple_callback handler;
    int partial_data;
} incomplete_t;

/* ========== Complex type combining everything ========== */
struct master_type {
    /* Scalars */
    scalar_int s_int;
    complex_scalar s_complex;
    vector_int s_vector;
    
    /* Strings */
    string_ptr str;
    
    /* Pointers */
    int_ptr iptr;
    opaque_ptr optr;
    generic_ptr gptr;
    
    /* Arrays */
    fixed_array fixed;
    incomplete_array* flex_ptr;  /* Pointer to incomplete array */
    
    /* Structs */
    struct simple_struct simple;
    struct packed_struct packed;
    struct aligned_struct aligned;
    struct recursive_struct recursive;
    
    /* Unions */
    union simple_union sunion;
    union complex_union cunion;
    
    /* Callbacks */
    simple_callback simple_cb;
    complex_callback complex_cb;
    
    /* Undefined/Forward reference resolved */
    struct opaque opaque_member;
    struct forward_declared_struct forward;
    
    /* Language-specific */
#ifdef __cplusplus
    struct cpp_like_struct cpp_struct;
#else
    struct transaction_struct trans_struct;
#endif
    
    /* Self-reference for graph complexity */
    struct master_type* self;
    struct master_type** friends;
    
    /* Variable length array as last member */
    int dynamic_data[];
};

/* ========== Function declarations using the types ========== */
void process_opaque(struct opaque* obj);
int traverse_recursive(struct recursive_struct* start);
void register_callback(complex_callback cb);
union complex_union create_complex_union(void);

#endif /* VARIED_TYPES_H */
