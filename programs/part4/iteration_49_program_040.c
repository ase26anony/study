#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct forward_declared_struct;
union forward_declared_union;

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
typedef __int128 my_int128_t;
typedef unsigned __int128 my_uint128_t;

/* String types (TYPE_STRING) */
typedef char* string_ptr;
typedef const char* const_string_ptr;

/* Function pointer types (TYPE_CALLBACK) */
typedef int (*simple_callback)(void);
typedef void (*complex_callback)(int, char*, ...);
typedef double (*math_callback)(double, double);
typedef void (*void_callback)(void);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef char char_array_20[20];
typedef double matrix_3x3[3][3];
typedef int (*func_ptr_array[5])(int);

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int x;
    double y;
    char z;
};

struct complex_struct {
    int id;
    char name[50];
    struct simple_struct nested;
    struct complex_struct* next;  /* Linked list */
    void (*operation)(struct complex_struct*);
} __attribute__((packed));

/* Union types (TYPE_UNION) */
union data_union {
    int as_int;
    double as_double;
    char as_string[20];
    void* as_pointer;
};

/* Tagged union */
struct tagged_union {
    enum { INT_TYPE, DOUBLE_TYPE, STRING_TYPE } type;
    union {
        int int_val;
        double double_val;
        char string_val[100];
    } value;
};

/* Anonymous struct/union */
struct container {
    int tag;
    union {
        struct {
            int x, y;
        } point;
        struct {
            float radius;
        } circle;
    } shape;
};

/* User struct (TYPE_USER_STRUCT) - using typedef */
typedef struct {
    int counter;
    char buffer[256];
    union {
        int flags;
        struct {
            unsigned int read : 1;
            unsigned int write : 1;
            unsigned int execute : 1;
        } perms;
    } access;
} user_struct_t;

/* Language struct (TYPE_LANG_STRUCT) - using GNU extensions */
struct lang_struct {
    int data __attribute__((aligned(16)));
    char flexible_array[];
} __attribute__((packed, aligned(32)));

/* Vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* Variadic types */
typedef va_list my_va_list;

/* Opaque pointer types */
typedef struct forward_declared_struct* opaque_ptr;

/* Function declarations */
void use_all_types(void);
extern void external_function(void*);

#endif /* TYPE_DEFS_H */
