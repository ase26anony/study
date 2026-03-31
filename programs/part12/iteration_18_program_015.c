#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========== SCALAR TYPES ========== */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern long long global_longlong;
extern _Bool global_bool;

/* ========== STRING TYPE ========== */
extern char *global_string;

/* ========== STRUCT TYPES ========== */
/* Regular struct */
struct SimpleStruct {
    int x;
    float y;
    char z;
};

/* Nested struct */
struct ComplexStruct {
    struct SimpleStruct inner;
    double extra;
    long long big;
};

/* ========== USER STRUCT TYPES (typedef'd) ========== */
typedef struct {
    int id;
    char name[32];
    float score;
} UserStruct;

typedef struct Node {
    int value;
    struct Node *next;
} LinkedListNode;

/* ========== UNION TYPES ========== */
/* Anonymous union */
union DataUnion {
    int i;
    float f;
    char str[16];
};

/* Typedef'd union */
typedef union {
    long long ll;
    double d;
    void *ptr;
} BigUnion;

/* ========== POINTER TYPES ========== */
extern int *int_ptr;
extern int **int_double_ptr;
extern struct SimpleStruct *struct_ptr;
extern UserStruct *user_struct_ptr;
extern void (*func_ptr)(void);

/* ========== ARRAY TYPES ========== */
extern int int_array[10];
extern struct SimpleStruct struct_array[5];
extern UserStruct user_struct_array[3];
extern int *pointer_array[8];

/* ========== CALLBACK TYPES ========== */
/* Function pointer typedef */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_id, void *data);

/* Function pointer variable */
extern void (*global_callback)(int, char *);

/* ========== LANGUAGE-SPECIFIC STRUCTS ========== */
/* GCC extension: transparent union */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    long *long_ptr;
} TransparentUnion;

/* Packed struct */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Vector type (GCC extension) */
typedef int v4si __attribute__ ((vector_size (16)));

/* ========== FUNCTION DECLARATIONS ========== */
void __attribute__((noinline)) use_scalar_types(int seed);
void __attribute__((noinline)) use_string_and_pointers(int seed);
void __attribute__((noinline)) use_structs_and_unions(int seed);
void __attribute__((noinline)) use_arrays_and_callbacks(int seed);
void __attribute__((noinline)) use_lang_specific_types(int seed);

/* Helper to prevent optimization */
int __attribute__((noinline)) opaque_operation(int x, int y);

#endif /* TYPE_ZOO_H */
