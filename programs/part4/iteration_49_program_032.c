#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct undefined_struct;
union undefined_union;

/* Scalar types (TYPE_SCALAR) */
typedef char my_char;
typedef short my_short;
typedef int my_int;
typedef long my_long;
typedef long long my_longlong;
typedef float my_float;
typedef double my_double;
typedef _Bool my_bool;
typedef _Complex float my_complex_float;
typedef _Complex double my_complex_double;
typedef __int128 my_int128 __attribute__((aligned(16)));

/* String type (TYPE_STRING) */
typedef const char* my_string;

/* Struct types (TYPE_STRUCT, TYPE_USER_STRUCT) */
struct simple_struct {
    int a;
    char b;
    float c;
};

struct __attribute__((packed)) packed_struct {
    int x;
    char y;
    double z;
} __attribute__((aligned(8)));

struct nested_struct {
    struct simple_struct inner;
    struct packed_struct* ptr;
};

/* Anonymous struct/union */
struct container {
    union {
        int as_int;
        float as_float;
    } anonymous_union;
    struct {
        char tag;
        void* data;
    } anonymous_struct;
};

/* Bitfield struct */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int : 4;  /* Padding */
    signed int value : 20;
};

/* Union types (TYPE_UNION) */
union simple_union {
    int i;
    float f;
    char* s;
};

union tagged_union {
    enum { INT_TAG, FLOAT_TAG, STRING_TAG } tag;
    struct {
        int type;
        union {
            int int_val;
            float float_val;
            char* string_val;
        };
    } data;
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef const int* const_int_ptr;
typedef volatile char* volatile_char_ptr;
typedef struct simple_struct* struct_ptr;
typedef union simple_union* union_ptr;

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef char char_array_2d[5][10];
typedef float float_array_3d[3][4][5];
typedef struct simple_struct struct_array[20];
typedef union simple_union union_array[15];

/* Callback/Function pointer types (TYPE_CALLBACK) */
typedef int (*simple_callback)(void);
typedef void (*callback_with_args)(int, char*, float);
typedef int (*callback_returning_ptr)(int**, char***);
typedef void (*variadic_callback)(int, ...);
typedef struct simple_struct* (*struct_returning_callback)(union simple_union*);

/* Array of function pointers */
typedef int (*func_ptr_array[5])(int, int);

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* Opaque handle */
typedef struct undefined_struct* opaque_handle;

/* Complex typedef chain */
typedef int chain1;
typedef chain1 chain2;
typedef chain2 chain3;
typedef chain3 chain4;
typedef chain4 chain5;

/* Language-specific struct placeholder (TYPE_LANG_STRUCT) */
struct lang_struct_placeholder {
    void* lang_specific;
};

/* Builtin types */
typedef __builtin_va_list my_va_list;

/* Function declarations using these types */
void use_all_types(void);
extern void external_func(struct undefined_struct*);

#endif /* TYPE_DEFS_H */
