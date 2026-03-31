#ifndef TYPES_H
#define TYPES_H

#include <stdarg.h>
#include <stddef.h>

/* Undefined/Incomplete Types (TYPE_UNDEFINED) */
struct undefined_struct;  // Forward declaration, never defined
union undefined_union;    // Forward declaration, never defined

/* Scalar Types (TYPE_SCALAR) */
typedef int scalar_int;
typedef char scalar_char;
typedef short scalar_short;
typedef long scalar_long;
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;
typedef _Complex float complex_float;
typedef _Complex double complex_double;
typedef __int128 int128_t;  // If available

/* String Types (TYPE_STRING) */
typedef char* string_ptr;
typedef const char* const_string_ptr;

/* Struct Types (TYPE_STRUCT, TYPE_USER_STRUCT) */
struct simple_struct {
    int a;
    char b;
    double c;
};

struct __attribute__((packed)) packed_struct {
    int x;
    char y;
    short z;
} __attribute__((aligned(8)));

struct nested_struct {
    struct simple_struct inner;
    struct packed_struct* packed_ptr;
};

struct with_bitfields {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int padding : 24;
};

/* Union Types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    char as_char[4];
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

/* Pointer Types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef struct simple_struct* struct_ptr;
typedef struct simple_struct** struct_ptr_ptr;

/* Array Types (TYPE_ARRAY) */
typedef int int_array[10];
typedef char char_array[20];
typedef struct simple_struct struct_array[5];
typedef int multi_dim_array[3][4][5];

/* Callback Types (TYPE_CALLBACK) */
typedef int (*simple_callback)(void);
typedef void (*complex_callback)(int, char*, ...);
typedef int (*math_callback)(int, int);
typedef void (*struct_callback)(struct simple_struct*);

/* Vector Types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Anonymous struct/union */
struct container {
    int id;
    union {
        int x;
        float y;
    };  // Anonymous union
    struct {
        char a;
        char b;
    };  // Anonymous struct
};

/* Linked list structure for type dependencies */
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

/* Function declarations */
void use_all_types(void);
void opaque_use(void* ptr);

/* Global variables to prevent elimination */
extern volatile int global_counter;

#endif /* TYPES_H */
