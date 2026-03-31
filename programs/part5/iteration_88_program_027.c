#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* ========== TYPE_UNDEFINED / Forward Declarations ========== */
struct opaque;                     /* TYPE_UNDEFINED initially */
struct forward_declared_struct;    /* Another undefined type */
typedef struct incomplete incomplete_t;

/* ========== TYPE_SCALAR ========== */
typedef int scalar_int;
typedef char scalar_char;
typedef _Bool scalar_bool;
typedef __complex__ double complex_scalar;
typedef __complex__ float complex_float;
typedef __builtin_va_list va_list_scalar;

/* Vector types (GCC extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* ========== TYPE_STRING ========== */
typedef const char* string_ptr;
typedef char* mutable_string;

/* ========== TYPE_STRUCT with various attributes ========== */
struct basic_struct {
    int x;
    double y;
    char z;
};

/* TYPE_USER_STRUCT with packed attribute */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(8)));

/* TYPE_USER_STRUCT with aligned attribute */
struct __attribute__((aligned(32))) aligned_struct {
    double data[4];
    long alignment_check;
};

/* Struct with designated initializer attribute */
struct __attribute__((designated_init)) designated_struct {
    int field1;
    char field2;
    float field3;
};

/* Struct with incomplete array (flexible array member) */
struct flexible_array {
    int count;
    int data[];  /* TYPE_ARRAY with unknown bound */
};

/* ========== TYPE_UNION ========== */
union basic_union {
    int as_int;
    float as_float;
    char as_chars[4];
    void* as_ptr;
};

/* Union with attributes */
union __attribute__((packed)) packed_union {
    long long big;
    char small[3];
};

/* ========== Recursive and Interconnected Types ========== */
/* Forward declaration now defined */
struct recursive_struct {
    int value;
    struct recursive_struct* next;  /* TYPE_POINTER to self */
    union basic_union data;
};

/* Mutual recursion between two structs */
struct type_a;
struct type_b;

struct type_a {
    int id;
    struct type_b* partner;  /* Pointer to forward-declared type */
};

struct type_b {
    int id;
    struct type_a* partner;
    struct type_a array_of_a[3];  /* TYPE_ARRAY of struct */
};

/* Opaque pointer type now defined */
struct opaque {
    void* hidden_data;
    int metadata;
    struct opaque* chain;  /* Chain of opaque pointers */
};

/* ========== TYPE_CALLBACK (function pointers) ========== */
typedef int (*simple_callback)(int, int);
typedef void (*complex_callback)(struct recursive_struct*, union basic_union);
typedef int (*va_callback)(int, ...);

/* Callback that takes a callback */
typedef void (*meta_callback)(simple_callback);

/* ========== TYPE_ARRAY variations ========== */
typedef int fixed_array[10];
typedef int* pointer_array[5];
typedef struct basic_struct struct_array[4];
typedef int (*callback_array[3])(void);

/* Multi-dimensional arrays */
typedef int matrix[3][3];
typedef struct basic_struct cube[2][2][2];

/* Incomplete array type in typedef */
typedef int incomplete_array[];

/* ========== TYPE_POINTER variations ========== */
typedef int* int_ptr;
typedef struct basic_struct* struct_ptr;
typedef union basic_union* union_ptr;
typedef void (*callback_ptr)(void);
typedef int (*func_ptr)(int);

/* Pointer to pointer */
typedef int** int_ptr_ptr;
typedef struct recursive_struct*** deep_ptr;

/* ========== C++ specific for TYPE_LANG_STRUCT ========== */
#ifdef __cplusplus
extern "C++" {
    /* This should generate TYPE_LANG_STRUCT in C++ mode */
    struct cpp_lang_struct {
        int cpp_field;
        virtual void method() = 0;
    };
    
    /* Transaction-safe struct (GCC extension) */
    struct __attribute__((transaction_safe)) transaction_struct {
        int safe_data;
        void update() __attribute__((transaction_safe));
    };
}
#else
/* For C mode, use other GCC extensions */
struct __attribute__((transaction_safe)) c_transaction_struct {
    int data;
};
#endif

/* ========== Complex nested type ========== */
struct master_container {
    struct recursive_struct recursive;
    union basic_union variant;
    struct basic_struct basic;
    fixed_array numbers;
    matrix grid;
    simple_callback cb;
    struct master_container* next;
    struct master_container* prev;
};

/* Union containing array of pointers */
union pointer_union {
    struct recursive_struct* struct_ptrs[5];
    void* generic_ptrs[10];
    int (*callbacks[3])(int);
};

/* ========== Function declarations using the types ========== */
void process_struct(struct basic_struct* s);
int use_callback(simple_callback cb, int a, int b);
struct recursive_struct* create_chain(int length);
void handle_opaque(struct opaque* op);

#endif /* VARIED_TYPES_H */
