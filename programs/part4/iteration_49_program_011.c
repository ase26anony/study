#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct undefined_struct;
union undefined_union;

/* Scalar types (TYPE_SCALAR) */
typedef int scalar_int;
typedef char scalar_char;
typedef short scalar_short;
typedef long scalar_long;
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;
typedef _Complex float complex_float;
typedef _Complex double complex_double;
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* String type (TYPE_STRING) */
typedef const char* string_type;

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int a;
    char b;
    float c;
    double d;
};

struct nested_struct {
    struct simple_struct inner;
    struct nested_struct *next;  /* Linked list */
    int data;
};

struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 24;
    int d : 16;
};

struct packed_struct {
    char a;
    int b;
    double c;
} __attribute__((packed));

struct aligned_struct {
    char a;
    int b __attribute__((aligned(16)));
    double c;
} __attribute__((aligned(32)));

/* Anonymous struct/union */
struct container {
    int tag;
    union {
        struct {
            int x;
            float y;
        } point;
        struct {
            char *name;
            int id;
        } data;
    } value;
};

/* User struct (TYPE_USER_STRUCT) - via typedef */
typedef struct {
    int id;
    char name[32];
    float score;
} user_struct_t;

/* Union types (TYPE_UNION) */
union simple_union {
    int i;
    float f;
    double d;
    char *s;
};

union tagged_union {
    int type;
    struct {
        int x, y;
    } point;
    struct {
        float radius;
    } circle;
    struct {
        int width, height;
    } rect;
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef const int* const_int_ptr;
typedef volatile int* volatile_int_ptr;
typedef const volatile int* const_volatile_int_ptr;

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef int int_array_2d[5][10];
typedef int int_array_3d[3][4][5];
typedef char* string_array[20];
typedef struct simple_struct struct_array[15];

/* Callback types (TYPE_CALLBACK) */
typedef int (*simple_callback)(void);
typedef void (*callback_with_args)(int, char*, float);
typedef int (*callback_returning_ptr)(char**, int);
typedef void (*variadic_callback)(int, ...);
typedef int (*complex_callback)(int (*)(int), void*);

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function pointer arrays */
typedef int (*func_ptr_array[5])(int, int);

/* Opaque incomplete type */
struct opaque_type;

/* External declarations */
extern struct simple_struct global_struct;
extern union simple_union global_union;
extern int_array_10 global_array;

/* Function declarations using all types */
void use_all_types(
    struct simple_struct *s,
    union simple_union *u,
    int_array_2d arr,
    simple_callback cb,
    v4si vec
);

/* Variadic function type */
typedef int (*va_callback)(va_list);

#endif /* TYPE_DEFS_H */
