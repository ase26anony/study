#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct *undefined_ptr_t;

/* Scalar types (TYPE_SCALAR) */
typedef char scalar_char;
typedef short scalar_short;
typedef int scalar_int;
typedef long scalar_long;
typedef long long scalar_long_long;
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;
typedef _Complex float scalar_complex_float;
typedef _Complex double scalar_complex_double;
#ifdef __SIZEOF_INT128__
typedef __int128 scalar_int128;
typedef unsigned __int128 scalar_uint128;
#endif

/* String types (TYPE_STRING) */
typedef const char *string_const_ptr;
typedef char *string_mutable_ptr;

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int a;
    char b;
    double c;
} __attribute__((packed));

struct nested_struct {
    struct simple_struct inner;
    struct nested_struct *next;
    int depth;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int padding : 24;
};

struct array_member_struct {
    int ids[10];
    char name[32];
    float matrix[3][3];
};

/* Anonymous struct/union */
struct anonymous_member_struct {
    struct {
        int x;
        int y;
    } point;
    union {
        int as_int;
        float as_float;
    } value;
};

/* User struct (TYPE_USER_STRUCT) - via typedef */
typedef struct {
    int id;
    char *name;
    void *data;
} user_struct_t;

/* Union types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    double as_double;
    void *as_ptr;
};

union tagged_union {
    enum { TAG_INT, TAG_FLOAT, TAG_STRING } tag;
    struct {
        int type;
        union {
            int i;
            float f;
            char *s;
        } value;
    } data;
};

/* Pointer types (TYPE_POINTER) */
typedef int *int_ptr_t;
typedef int **int_double_ptr_t;
typedef int ***int_triple_ptr_t;
typedef const int *const_int_ptr_t;
typedef volatile int *volatile_int_ptr_t;
typedef const volatile int *const_volatile_int_ptr_t;

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef int int_array_2d[5][5];
typedef int int_array_3d[3][3][3];
typedef char *string_array[20];
typedef void (*func_ptr_array[5])(void);

/* Callback types (TYPE_CALLBACK) */
typedef int (*simple_callback_t)(void);
typedef void (*complex_callback_t)(int, char *, ...);
typedef int (*math_callback_t)(double, double);
typedef void (*void_callback_t)(void *context, int param);

/* Language struct (TYPE_LANG_STRUCT) - using va_list */
typedef struct {
    va_list args;
    int count;
    const char *format;
} lang_struct_t;

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Complex type relationships */
typedef struct list_node {
    void *data;
    struct list_node *next;
    struct list_node *prev;
} list_node_t;

typedef struct tree_node {
    int value;
    struct tree_node *left;
    struct tree_node *right;
    struct tree_node *parent;
} tree_node_t;

typedef struct graph_node {
    int id;
    struct graph_node **neighbors;
    int neighbor_count;
} graph_node_t;

/* Function declarations */
void use_all_types(void);
void opaque_use(void *ptr);

#endif /* TYPE_DEFS_H */
