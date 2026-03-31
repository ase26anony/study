/* type_zoo.h - Header file with type declarations */
#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

#include <stddef.h>

/* Scalar types (TYPE_SCALAR) */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern long long global_longlong;
extern _Bool global_bool;

/* String type (TYPE_STRING) */
extern char *global_string;

/* Struct types (TYPE_STRUCT) */
struct SimpleStruct {
    int x;
    float y;
};

struct ComplexStruct {
    struct SimpleStruct nested;
    double extra;
    char tag;
};

/* User struct types (TYPE_USER_STRUCT) */
typedef struct {
    int id;
    char name[32];
    float score;
} UserStruct;

typedef struct Node {
    int value;
    struct Node *next;
} ListNode;

/* Union types (TYPE_UNION) */
union DataUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

typedef union {
    long long big;
    double precise;
} TypedUnion;

/* Pointer types (TYPE_POINTER) */
extern int *int_ptr;
extern struct SimpleStruct *struct_ptr;
extern union DataUnion *union_ptr;
extern void **void_double_ptr;

/* Array types (TYPE_ARRAY) */
extern int int_array[10];
extern struct SimpleStruct struct_array[5];
extern char *string_array[8];

/* Callback types (TYPE_CALLBACK) */
typedef int (*Comparator)(const void *, const void *);
typedef void (*SimpleCallback)(int, float);

/* Language-specific struct (TYPE_LANG_STRUCT) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    void *void_ptr;
} TransparentUnion;

/* Function declarations */
void use_scalars(void);
void use_structs_and_unions(void);
void use_pointers_and_arrays(void);
void use_callbacks(void);
void use_lang_structs(void);

#endif /* TYPE_ZOO_H */
