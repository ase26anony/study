/* type_zoo.h - Header file with type declarations */
#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

#include <stddef.h>

/* Scalar types */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef char scalar_char_t;
typedef long long scalar_ll_t;
typedef _Bool scalar_bool_t;

/* String type */
typedef char* string_ptr_t;

/* Struct types */
struct SimpleStruct {
    int x;
    float y;
    char z;
};

struct ComplexStruct {
    struct SimpleStruct nested;
    double extra;
    long long big;
};

/* User struct types (typedef'd structs) */
typedef struct {
    int id;
    char name[32];
    float score;
} UserStruct;

typedef struct Node {
    int value;
    struct Node* next;
} ListNode;

/* Union types */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char;
};

typedef union {
    long long as_ll;
    double as_double;
    void* as_ptr;
} TypedefUnion;

/* Pointer types */
typedef int* int_ptr_t;
typedef int** int_ptr_ptr_t;
typedef struct SimpleStruct* struct_ptr_t;
typedef void (*generic_callback_t)(void);

/* Array types */
typedef int int_array_10_t[10];
typedef struct SimpleStruct struct_array_5_t[5];
typedef int* pointer_array_20_t[20];

/* Callback types */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(const char*, void*);
typedef void (*no_args_callback)(void);

/* Language-specific struct (using GCC extensions) */
#ifdef __GNUC__
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    int int_member;
    float float_member;
};
#endif

/* Function declarations */
void use_scalar_types(void);
void use_string_types(void);
void use_struct_types(void);
void use_union_types(void);
void use_pointer_types(void);
void use_array_types(void);
void use_callback_types(void);
void use_lang_struct_types(void);

/* Global variables for type visibility */
extern scalar_int_t global_int;
extern scalar_float_t global_float;
extern string_ptr_t global_string;
extern struct SimpleStruct global_struct;
extern UserStruct global_user_struct;
extern union SimpleUnion global_union;
extern int_ptr_t global_int_ptr;
extern int_array_10_t global_int_array;
extern simple_callback global_callback;

#endif /* TYPE_ZOO_H */
