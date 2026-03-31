#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct undefined_struct;
union undefined_union;

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

/* String type (TYPE_STRING) */
typedef char* string_ptr;

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int a;
    char b;
    float c;
    double d;
};

struct nested_struct {
    struct simple_struct inner;
    long extra;
};

struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    int d : 16;
};

struct array_member_struct {
    int arr[10];
    float matrix[3][3];
    char* strings[5];
};

/* Packed struct with alignment */
struct __attribute__((packed, aligned(8))) packed_struct {
    char a;
    int b;
    short c;
};

/* Union types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    char* as_string;
};

union tagged_union {
    enum { TAG_INT, TAG_FLOAT, TAG_STRING } tag;
    struct {
        int type;
        union {
            int i;
            float f;
            char* s;
        } value;
    } data;
};

/* Anonymous struct/union */
struct container {
    int id;
    union {
        struct {
            int x, y;
        } point;
        struct {
            float radius;
        } circle;
    } shape;
};

/* User struct (TYPE_USER_STRUCT) - via typedef */
typedef struct {
    int data;
    void* next;
} user_struct_t;

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef struct simple_struct* struct_ptr;
typedef void (*void_func_ptr)(void);
typedef int (*int_func_ptr)(int, int);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef float float_matrix_3x3[3][3];
typedef char* string_array[5];
typedef int (*func_ptr_array[5])(void);

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op)(int, int);
typedef void (*callback)(void* data, int result);
typedef char* (*string_mapper)(const char*);
typedef void (*varargs_func)(int, ...);

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function pointer with complex signature */
typedef void* (*allocator)(size_t size, void* context);
typedef void (*deallocator)(void* ptr, void* context);

/* Opaque handle */
typedef struct opaque_handle* handle_t;

/* Language struct placeholder (TYPE_LANG_STRUCT) */
/* This would typically be GCC internal types */

/* External declarations */
extern volatile int global_counter;
extern struct simple_struct global_struct;

/* Function declarations */
void use_all_types(void);
void* opaque_function(void* ptr);

#endif /* TYPE_DEFS_H */
