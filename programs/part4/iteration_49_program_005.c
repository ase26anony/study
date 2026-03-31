#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct *undefined_ptr_t;

/* Scalar types (TYPE_SCALAR) */
typedef char char_type;
typedef short short_type;
typedef int int_type;
typedef long long_type;
typedef float float_type;
typedef double double_type;
typedef _Bool bool_type;
typedef __int128 int128_type;
typedef _Complex float complex_float;
typedef _Complex double complex_double;

/* String type (TYPE_STRING) */
typedef const char *string_type;

/* Struct types (TYPE_STRUCT, TYPE_USER_STRUCT) */
struct simple_struct {
    int a;
    char b;
    float c;
};

struct complex_struct {
    struct simple_struct nested;
    struct complex_struct *next;  /* Linked list */
    void (*callback)(int);        /* Function pointer member */
    volatile int volatile_member;
    unsigned int bitfield:4;
    unsigned int another_bit:8;
} __attribute__((packed, aligned(16)));

/* Anonymous struct/union */
struct with_anonymous {
    union {
        int as_int;
        float as_float;
    };
    struct {
        char x;
        char y;
    };
};

/* Union types (TYPE_UNION) */
union data_union {
    int i;
    float f;
    double d;
    char *str;
};

/* Tagged union */
struct tagged_union {
    enum { INT, FLOAT, STRING } tag;
    union {
        int int_val;
        float float_val;
        char *string_val;
    } value;
};

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef char char_matrix[5][10];
typedef float three_d_array[3][4][5];

/* Function pointer types (TYPE_CALLBACK) */
typedef int (*binary_op)(int, int);
typedef void (*void_func)(void);
typedef char *(*string_transform)(const char *, int);
typedef int (*variadic_func)(int, ...);

/* Language-specific struct (TYPE_LANG_STRUCT) */
typedef __builtin_va_list va_list_type;

/* Vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Opaque handle */
typedef struct opaque_handle *handle_t;

/* Complex type graph */
struct node {
    int value;
    struct node *left;
    struct node *right;
    struct node **parents;  /* Array of pointers */
    void (*visitor)(struct node *);
};

/* Self-referential types */
struct recursive {
    int data;
    struct recursive *next;
    union {
        struct recursive *alt_next;
        void *generic;
    } link;
};

/* External declarations */
extern struct simple_struct global_struct;
extern union data_union global_union;
extern int_array_10 global_array;

/* Function declarations */
void use_all_types(void);
void opaque_use(void *ptr);

#endif /* TYPE_DEFS_H */
