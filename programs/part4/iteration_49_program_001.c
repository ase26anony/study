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
typedef __int128 my_int128;  /* If supported */

/* String types (TYPE_STRING) */
typedef char* my_string;
typedef const char* my_const_string;

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int a;
    char b;
    float c;
    double d;
};

struct complex_struct {
    int id;
    char name[32];
    float values[16];
    struct simple_struct nested;
    struct complex_struct* next;  /* Self-referential */
    void (*callback)(int);        /* Function pointer member */
};

/* Packed struct with bitfields */
struct __attribute__((packed)) packed_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int value : 24;
    char padding;
};

/* Anonymous struct within struct */
struct container_struct {
    int type;
    union {
        int int_value;
        float float_value;
        char* string_value;
    } data;
    struct {
        int x, y, z;
    } coordinates;
};

/* User struct (TYPE_USER_STRUCT) - via typedef */
typedef struct {
    int x;
    int y;
    int width;
    int height;
} rectangle_t;

/* Another user struct with attributes */
typedef struct __attribute__((aligned(16))) aligned_struct {
    double data[2];
    long alignment_enforcer;
} aligned_struct_t;

/* Union types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    char as_char[4];
    void* as_ptr;
};

/* Tagged union */
union tagged_union {
    struct {
        int type;
        union {
            int int_val;
            double double_val;
            char* str_val;
        };
    };
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_double_ptr;
typedef int*** int_triple_ptr;
typedef struct simple_struct* struct_ptr;
typedef struct simple_struct** struct_double_ptr;
typedef void (*void_func_ptr)(void);
typedef int (*int_func_ptr)(int, int);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef int int_array_2d[5][10];
typedef int int_array_3d[3][4][5];
typedef struct simple_struct struct_array[8];
typedef void (*func_ptr_array[5])(void);

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op)(int, int);
typedef void (*event_handler)(void* context, int event_id);
typedef char* (*string_formatter)(const char* format, ...);
typedef int (*va_func)(int count, ...);

/* Language struct (TYPE_LANG_STRUCT) - using builtin types */
typedef __builtin_va_list va_list_type;
typedef __SIZE_TYPE__ size_type;

/* Vector types (GNU extension) */
typedef int v4si __attribute__ ((vector_size (16)));
typedef float v4sf __attribute__ ((vector_size (16)));

/* Opaque function declarations to prevent optimization */
void use_int_ptr(int* p);
void use_struct_ptr(struct simple_struct* p);
void use_union_ptr(union simple_union* p);
void use_func_ptr(void (*fp)(void));
void use_va_list(va_list_type args);

#endif /* TYPE_DEFS_H */
