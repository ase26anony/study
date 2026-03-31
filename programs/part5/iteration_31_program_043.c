#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declaration without definition */
struct undefined_struct;

/* ==================== TYPE_SCALAR ==================== */
/* Basic scalar types - these are implicitly GTY-marked when used in GTY structs */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;

/* ==================== TYPE_STRING ==================== */
/* String pointer type */
typedef const char *GTY(()) string_t;

/* ==================== TYPE_STRUCT ==================== */
/* Regular struct types */
struct GTY(()) simple_struct {
    int x;
    float y;
};

struct GTY(()) nested_struct {
    struct simple_struct inner;
    double extra;
};

/* Struct with pointer member */
struct GTY(()) struct_with_ptr {
    int value;
    struct simple_struct *GTY((skip)) ptr;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* Typedef'd struct - becomes TYPE_USER_STRUCT */
typedef struct GTY(()) {
    int id;
    char name[32];
} user_struct_t;

/* Another user struct with function pointer */
typedef struct GTY(()) {
    int counter;
    void (*GTY((skip)) callback)(int);
} user_struct_with_callback_t;

/* ==================== TYPE_UNION ==================== */
/* Union type */
union GTY(()) data_union {
    int int_val;
    float float_val;
    double double_val;
    char *GTY((skip)) string_val;
};

/* Typedef'd union */
typedef union GTY(()) {
    long long_val;
    void *GTY((skip)) ptr_val;
} union_t;

/* ==================== TYPE_POINTER ==================== */
/* Various pointer typedefs */
typedef int *GTY((skip)) int_ptr_t;
typedef struct simple_struct *GTY((skip)) struct_ptr_t;
typedef union data_union *GTY((skip)) union_ptr_t;

/* Pointer to pointer */
typedef int **GTY((skip)) int_ptr_ptr_t;

/* ==================== TYPE_ARRAY ==================== */
/* Array typedefs */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef char GTY(()) char_array_t[256];

/* Array of pointers */
typedef int *GTY((skip)) ptr_array_t[8];

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedefs */
typedef void (*GTY((skip)) simple_callback_t)(int);
typedef int (*GTY((skip)) complex_callback_t)(const char *, int, void *);

/* Callback returning pointer */
typedef struct simple_struct *(*GTY((skip)) struct_factory_t)(int);

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structs with GTY markers */
struct GTY((desc("%0.type"), tag("lang_type"))) lang_struct {
    int type;
    union {
        int int_val;
        double float_val;
    } GTY((desc("%1.type"))) u;
};

/* Another lang struct with chain next pointer */
struct GTY((chain_next("%h.next"))) lang_chain_struct {
    int value;
    struct lang_chain_struct *GTY((skip)) next;
};

/* ==================== COMPLEX NESTED TYPES ==================== */
/* Struct containing array of pointers to unions */
struct GTY(()) container_struct {
    int id;
    union data_union *GTY((skip)) items[4];
    user_struct_t metadata;
};

/* Union containing struct with callback */
union GTY(()) complex_union {
    struct GTY(()) {
        int x;
        simple_callback_t GTY((skip)) handler;
    } s;
    int_array_t array;
};

/* Include auxiliary types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
