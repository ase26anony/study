/* types.h - Header file with type declarations */
#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

/* Scalar types */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef char scalar_char;
typedef long long scalar_llong;
typedef _Bool scalar_bool;

/* String type */
typedef char* string_type;

/* Struct types */
struct SimpleStruct {
    int a;
    float b;
    char c;
};

struct ComplexStruct {
    struct SimpleStruct nested;
    double extra;
    long long big;
};

/* User struct types (typedef'd structs) */
typedef struct {
    int x;
    int y;
    int z;
} UserStruct3D;

typedef struct Node {
    int value;
    struct Node* next;
} LinkedListNode;

/* Union types */
union BasicUnion {
    int as_int;
    float as_float;
    char as_char;
};

typedef union {
    long long as_llong;
    double as_double;
    void* as_ptr;
} TypedefUnion;

/* Pointer types */
typedef int* int_ptr;
typedef int** int_dbl_ptr;
typedef struct SimpleStruct* struct_ptr;
typedef void (*generic_callback)(void);

/* Array types */
typedef int int_array[10];
typedef struct SimpleStruct struct_array[5];
typedef int* pointer_array[8];

/* Callback types */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(const char*, void*);
typedef void (*no_args_callback)(void);

/* Language-specific struct (GCC extensions) */
#ifdef __GNUC__
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

union __attribute__((transparent_union)) TransparentUnion {
    int* int_ptr;
    void* void_ptr;
};
#endif

/* Function declarations */
void use_scalars(void);
void use_strings(void);
void use_structs(void);
void use_unions(void);
void use_pointers(void);
void use_arrays(void);
void use_callbacks(void);
void use_lang_structs(void);

#endif /* TYPES_H */
