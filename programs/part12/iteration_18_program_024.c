#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* ========== Scalar Types ========== */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern long long global_long_long;
extern _Bool global_bool;

/* ========== String Type ========== */
extern char *global_string;

/* ========== Struct Types ========== */
/* Plain struct */
struct PlainStruct {
    int x;
    float y;
    char z;
};

/* Nested struct */
struct OuterStruct {
    struct PlainStruct inner;
    double extra;
    long long big;
};

/* Struct with array member */
struct ArrayStruct {
    int data[10];
    char name[32];
};

/* ========== User Struct Types (typedef'd) ========== */
typedef struct {
    int id;
    float score;
    char name[20];
} UserStruct;

typedef struct Node {
    int value;
    struct Node *next;
} LinkedList;

/* ========== Union Types ========== */
/* Anonymous union */
union AnonymousUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

/* Typedef'd union */
typedef union {
    long long timestamp;
    double precision_time;
    struct {
        int seconds;
        int nanoseconds;
    } parts;
} TimeUnion;

/* ========== Pointer Types ========== */
extern int *global_int_ptr;
extern int **global_int_ptr_ptr;
extern struct PlainStruct *global_struct_ptr;
extern UserStruct **global_user_struct_ptr_ptr;
extern void *global_void_ptr;

/* ========== Array Types ========== */
extern int global_int_array[20];
extern struct PlainStruct global_struct_array[5];
extern int *global_pointer_array[8];

/* ========== Callback Types ========== */
/* Function pointer declaration */
extern void (*global_callback)(int, char*);

/* Typedef for function pointer */
typedef int (*Comparator)(const void*, const void*);
extern Comparator global_comparator;

/* More complex callback */
typedef void (*EventHandler)(int event_id, void *user_data);
extern EventHandler global_event_handler;

/* ========== Language-Specific Structs ========== */
/* GCC extension: transparent union */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    void *void_ptr;
} TransparentUnion;

/* Packed struct */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Vector type (GCC extension) */
typedef int v4si __attribute__((vector_size(16)));

/* ========== Function Declarations ========== */
void __attribute__((noinline)) use_scalar_types(volatile int seed);
void __attribute__((noinline)) use_struct_types(volatile int seed);
void __attribute__((noinline)) use_union_types(volatile int seed);
void __attribute__((noinline)) use_pointer_types(volatile int seed);
void __attribute__((noinline)) use_array_types(volatile int seed);
void __attribute__((noinline)) use_callback_types(volatile int seed);
void __attribute__((noinline)) use_lang_structs(volatile int seed);

/* Callback function implementations */
int compare_ints(const void *a, const void *b);
void sample_callback(int param, char *msg);
void event_handler_impl(int event_id, void *user_data);

#endif /* TYPE_ZOO_H */
