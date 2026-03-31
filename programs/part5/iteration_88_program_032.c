#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;

/* TYPE_SCALAR: Various scalar typedefs */
typedef int scalar_int;
typedef char scalar_char;
typedef _Bool scalar_bool;
typedef __complex__ double complex_scalar;
typedef int __attribute__((vector_size(16))) vector_scalar;
typedef __builtin_va_list va_list_scalar;

/* TYPE_STRING: String types */
typedef const char* string_ptr;
typedef char* mutable_string;

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef void* generic_ptr;
typedef struct opaque* opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef int incomplete_array[];
typedef int zero_array[0];
typedef int (*array_of_ptrs)[5];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*simple_callback)(void);
typedef void (*complex_callback)(int, const char*, va_list);
typedef struct forward_declared* (*struct_callback)(int);

/* TYPE_UNION: Union definitions */
union simple_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

union nested_union {
    struct {
        int x;
        int y;
    } point;
    struct {
        void* data;
        int size;
    } buffer;
};

/* TYPE_STRUCT: Regular struct definitions */
struct recursive_struct {
    int value;
    struct recursive_struct* next;  /* Self-reference */
    struct forward_declared* forward_ref;
};

struct with_arrays {
    fixed_array fixed;
    int* dynamic;
    incomplete_array flex;  /* Must be last member */
};

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed, aligned(2))) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((designated_init));

struct __attribute__((aligned(64))) aligned_struct {
    double data[8];
    long long metadata;
};

/* Complete the forward declared struct */
struct forward_declared {
    int id;
    struct recursive_struct* recursive;
    union simple_union data;
};

/* Opaque struct definition (completes TYPE_UNDEFINED) */
struct opaque {
    int secret;
    char* name;
    struct opaque* next;
};

/* Complex nested type definitions */
typedef struct {
    union nested_union data;
    struct aligned_struct alignment;
    complex_callback callback;
} nested_container;

/* Array of function pointers */
typedef void (*func_array[5])(void);

/* TYPE_LANG_STRUCT: C++ specific (if compiled as C++) */
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
        int value;
        void update(int new_val) __attribute__((transaction_safe));
    };
}
#endif

/* GNU extension structs */
struct __attribute__((scalar_storage_order("big-endian"))) endian_struct {
    int a;
    short b;
};

/* Variable length struct (GNU extension) */
struct vla_struct {
    int size;
    int data[];  /* Flexible array member */
};

/* Callback that uses all major types */
typedef struct recursive_struct* (*universal_callback)(
    struct with_arrays*,
    union simple_union,
    complex_scalar,
    va_list
);

#endif /* VARIED_TYPES_H */
