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
    int x;
};

struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 24;
    int d : 1;
};

/* User struct (TYPE_USER_STRUCT) */
typedef struct {
    int id;
    char name[32];
    float score;
} user_struct_t;

/* Union types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    char as_char[4];
};

union tagged_union {
    int type;
    struct {
        int type;
        int value;
    } int_data;
    struct {
        int type;
        double value;
    } double_data;
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef struct simple_struct* struct_ptr;
typedef void (*void_func_ptr)(void);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef char char_array_2d[5][10];
typedef struct simple_struct struct_array[3];
typedef int (*func_ptr_array[5])(int, int);

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op)(int, int);
typedef void (*callback_func)(void* data, int result);
typedef int (*variadic_func)(int, ...);

/* Language struct (TYPE_LANG_STRUCT) - using va_list */
typedef struct {
    va_list args;
    int count;
} lang_struct_t;

/* Complex type relationships */
struct tree_node {
    int value;
    struct tree_node* left;
    struct tree_node* right;
    void (*print)(struct tree_node*);
};

struct linked_list {
    void* data;
    struct linked_list* next;
    struct linked_list* prev;
};

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Anonymous struct/union */
struct container {
    union {
        int x;
        float y;
    };
    struct {
        char a;
        char b;
    };
};

/* Function pointer with complex signature */
typedef void* (*allocator_func)(size_t size, void* context);
typedef void (*deallocator_func)(void* ptr, void* context);

/* Opaque handle */
typedef struct opaque_handle* handle_t;

/* Extern declarations for multi-file testing */
extern struct simple_struct global_struct;
extern union simple_union global_union;
extern int global_array[20];

/* Function declarations */
void use_all_types(void);
void* opaque_function(void* ptr);

#endif /* TYPE_DEFS_H */
