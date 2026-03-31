#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Undefined/Incomplete types (TYPE_UNDEFINED) */
struct forward_declared_struct;  /* Never defined */
union forward_declared_union;    /* Never defined */

/* Scalar types (TYPE_SCALAR) */
typedef int my_int;
typedef char my_char;
typedef short my_short;
typedef long my_long;
typedef float my_float;
typedef double my_double;
typedef _Bool my_bool;
typedef _Complex float my_complex_float;
typedef _Complex double my_complex_double;
typedef __int128 my_int128;  /* GNU extension */

/* String type (TYPE_STRING) */
typedef char* my_string;

/* Struct types (TYPE_STRUCT, TYPE_USER_STRUCT) */
struct simple_struct {
    int a;
    char b;
    float c;
};

struct __attribute__((packed)) packed_struct {
    int x;
    char y;
    double z __attribute__((aligned(8)));
};

struct nested_struct {
    struct simple_struct inner;
    struct packed_struct* ptr_to_packed;
};

/* Anonymous struct/union */
struct with_anonymous {
    union {
        int as_int;
        float as_float;
    };
    struct {
        char tag;
        char data[16];
    };
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
    enum { INT, FLOAT, STRING } tag;
    struct {
        int type;
        union {
            int int_val;
            float float_val;
            char* str_val;
        };
    };
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef struct simple_struct* struct_ptr;
typedef struct simple_struct** struct_ptr_ptr;
typedef void (*void_func_ptr)(void);
typedef int (*int_func_ptr)(int, char*);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef char char_array_5x5[5][5];
typedef float float_array_3d[3][3][3];
typedef struct simple_struct struct_array[4];
typedef void_func_ptr func_ptr_array[8];

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op)(int, int);
typedef void (*callback_with_va)(const char*, ...);
typedef void (*error_handler)(int, const char*, ...);
typedef struct simple_struct* (*struct_factory)(void);
typedef union simple_union (*union_transformer)(union simple_union);

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Language struct placeholder (TYPE_LANG_STRUCT) */
/* This would typically be GCC internal types */
typedef __builtin_va_list va_list_type;

/* Complex type relationships */
struct tree_node {
    int value;
    struct tree_node* left;
    struct tree_node* right;
    struct tree_node* parent;
};

struct linked_list {
    void* data;
    struct linked_list* next;
    struct linked_list* prev;
};

struct graph_node {
    int id;
    struct graph_node** neighbors;
    int neighbor_count;
};

/* Function pointer with complex signature */
typedef void* (*allocator_func)(size_t, void* context);
typedef void (*deallocator_func)(void*, void* context);

struct memory_manager {
    allocator_func alloc;
    deallocator_func free;
    void* context;
};

/* Opaque handle */
typedef struct opaque_handle* handle_t;

#endif /* TYPE_DEFS_H */
