#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct forward_declared_struct;
union forward_declared_union;

/* Scalar types (TYPE_SCALAR) */
typedef int scalar_int;
typedef char scalar_char;
typedef short scalar_short;
typedef long scalar_long;
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;
typedef _Complex float complex_float;
typedef _Complex double complex_double;
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* String types (TYPE_STRING) */
typedef char* string_ptr;
typedef const char* const_string_ptr;

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int a;
    char b;
    double c;
} __attribute__((packed));

struct nested_struct {
    struct simple_struct inner;
    struct forward_declared_struct* forward_ptr;
    int data;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int reserved : 24;
} __attribute__((packed, aligned(8)));

/* Anonymous struct/union */
struct container {
    struct {
        int x;
        int y;
    } point;
    union {
        int as_int;
        float as_float;
    } value;
};

/* Union types (TYPE_UNION) */
union simple_union {
    int i;
    float f;
    char* s;
};

union tagged_union {
    enum { INT, FLOAT, STRING } tag;
    struct {
        int type;
        union {
            int i;
            float f;
            char* s;
        } value;
    } data;
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_double_ptr;
typedef int*** int_triple_ptr;
typedef struct simple_struct* struct_ptr;
typedef struct forward_declared_struct* forward_ptr;
typedef void (*void_func_ptr)(void);
typedef int (*int_func_ptr)(int, char*);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef char char_array_2d[5][10];
typedef struct simple_struct struct_array[3];
typedef void (*func_ptr_array[5])(void);

/* Callback types (TYPE_CALLBACK) */
typedef int (*callback_int)(int, int);
typedef void (*callback_void)(void*);
typedef char* (*callback_string)(const char*, ...);
typedef int (*callback_va)(int, va_list);

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function pointer with complex signature */
typedef union tagged_union* (*complex_callback)(
    struct nested_struct*,
    int_array_10,
    callback_int
);

/* Opaque handle */
typedef struct opaque_handle* handle_t;

/* Language struct placeholder (TYPE_LANG_STRUCT) */
struct lang_struct_marker {
    void* lang_specific;
};

/* External declarations to force cross-file type resolution */
extern struct cross_file_struct;
extern union cross_file_union;

#endif /* TYPE_DEFS_H */
