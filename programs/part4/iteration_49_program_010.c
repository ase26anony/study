#ifndef TYPES_H
#define TYPES_H

#include <stdarg.h>

/* Undefined/Incomplete types */
struct undefined_struct;
union undefined_union;

/* Scalar types */
typedef int scalar_int;
typedef char scalar_char;
typedef short scalar_short;
typedef long scalar_long;
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;

/* GNU extensions */
typedef __int128 scalar_int128 __attribute__((aligned(16)));
typedef _Complex float scalar_complex_float;
typedef _Complex double scalar_complex_double;

/* Vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* String type */
typedef char* string_ptr;

/* Basic struct types */
struct basic_struct {
    int a;
    char b;
    double c;
    scalar_int128 d;
};

/* Packed struct */
struct packed_struct {
    int a;
    char b;
    double c;
} __attribute__((packed));

/* Struct with bitfields */
struct bitfield_struct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 16;
    unsigned int d : 4;
};

/* Nested struct */
struct outer_struct {
    struct basic_struct inner;
    struct outer_struct* self_ptr;
    int data;
};

/* Union types */
union basic_union {
    int i;
    float f;
    double d;
    char* s;
};

/* Tagged union */
struct tagged_union {
    enum { INT, FLOAT, STRING } tag;
    union {
        int i;
        float f;
        char* s;
    } value;
};

/* Pointer types */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef struct basic_struct* struct_ptr;
typedef struct basic_struct** struct_ptr_ptr;

/* Array types */
typedef int int_array[10];
typedef int multi_array[5][10][15];
typedef struct basic_struct struct_array[20];
typedef int (*func_ptr_array[5])(int, int);

/* Callback/Function pointer types */
typedef int (*simple_callback)(int, int);
typedef void (*complex_callback)(struct basic_struct*, union basic_union*, ...);
typedef int (*variadic_callback)(int, ...);

/* Typedef chains */
typedef int chain1;
typedef chain1 chain2;
typedef chain2 chain3;
typedef chain3 chain4;

/* Anonymous struct/union */
struct container {
    int type;
    union {
        struct {
            int x, y;
        } point;
        struct {
            float radius;
        } circle;
    } shape;
};

/* Linked list structure */
struct list_node {
    int data;
    struct list_node* next;
    struct list_node* prev;
};

/* Tree structure */
struct tree_node {
    int value;
    struct tree_node* left;
    struct tree_node* right;
    struct tree_node* parent;
};

/* Complex interdependent types */
struct type_a;
struct type_b;

struct type_a {
    int id;
    struct type_b* b_ptr;
    struct type_a* next;
};

struct type_b {
    int id;
    struct type_a* a_ptr;
    struct type_b* (*processor)(struct type_a*);
};

/* Function declarations */
void use_all_types(void);
extern void external_function(void*);

#endif /* TYPES_H */
