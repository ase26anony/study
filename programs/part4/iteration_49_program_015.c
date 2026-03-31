#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>
#include <stddef.h>

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

/* Callback types (TYPE_CALLBACK) */
typedef int (*simple_callback)(void);
typedef void (*complex_callback)(int, char*, ...);
typedef double (*math_callback)(double, double);
typedef void (*void_callback)(void);

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_double_ptr;
typedef int*** int_triple_ptr;
typedef void (*func_ptr)(void);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef char char_array_5x5[5][5];
typedef double double_array_3d[3][3][3];

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int x;
    double y;
    char z;
};

struct complex_struct {
    int id;
    char name[50];
    struct simple_struct* nested;
    void (*operation)(struct complex_struct*);
};

struct packed_struct {
    char a;
    int b __attribute__((packed));
    double c;
} __attribute__((packed, aligned(8)));

struct with_bitfields {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int reserved : 24;
};

struct with_anonymous_union {
    int type;
    union {
        int int_value;
        double double_value;
        char* string_value;
    } data;
};

/* User struct types (TYPE_USER_STRUCT) */
typedef struct simple_struct SimpleStruct;
typedef struct complex_struct ComplexStruct;

/* Union types (TYPE_UNION) */
union data_union {
    int int_val;
    float float_val;
    double double_val;
    char* string_val;
    void* ptr_val;
};

union tagged_union {
    enum { INT, FLOAT, STRING, ARRAY } tag;
    struct {
        int type;
        union {
            int i;
            float f;
            char* s;
            int arr[5];
        } value;
    } data;
};

/* Vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Language struct placeholder (TYPE_LANG_STRUCT) */
/* This would typically be GCC internal types */

/* Function prototypes */
void use_all_types(void);
void opaque_use(void* ptr);

#endif /* TYPE_DEFS_H */
