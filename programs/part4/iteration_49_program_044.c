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
typedef __int128 my_int128;  /* If available */

/* String type (TYPE_STRING) */
typedef char* my_string;

/* Struct types (TYPE_STRUCT, TYPE_USER_STRUCT) */
struct simple_struct {
    int a;
    char b;
    double c;
};

struct __attribute__((packed)) packed_struct {
    int x;
    char y;
    long z;
} __attribute__((aligned(16)));

struct complex_struct {
    int id;
    char name[32];
    struct simple_struct nested;
    struct complex_struct* next;  /* Linked list */
    void (*callback)(int);        /* Function pointer member */
};

/* Anonymous struct/union */
struct container {
    int type;
    union {
        int int_val;
        double dbl_val;
        char* str_val;
    } data;
    struct {
        unsigned int flags : 4;
        unsigned int mode : 3;
    } bits;
};

/* Union types (TYPE_UNION) */
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

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef struct simple_struct* struct_ptr;
typedef void (*void_func_ptr)(void);
typedef int (*complex_func_ptr)(int, char*, ...);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef char char_array_2d[5][10];
typedef struct simple_struct struct_array[3];
typedef void (*func_ptr_array[5])(void);

/* Callback types (TYPE_CALLBACK) */
typedef void (*simple_callback)(void);
typedef int (*math_callback)(int, int);
typedef char* (*string_callback)(const char*, ...);
typedef void (*va_callback)(int, va_list);

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Language-specific struct placeholder (TYPE_LANG_STRUCT) */
/* This would typically be GCC internal types, but we can hint at it */
struct __attribute__((transaction_safe)) transaction_struct {
    int value;
    void* ptr;
};

/* Function declarations */
void use_all_types(void);
extern void opaque_function(void*);

#endif /* TYPE_DEFS_H */
