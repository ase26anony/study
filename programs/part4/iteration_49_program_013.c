#ifndef TYPES_H
#define TYPES_H

#include <stdarg.h>
#include <stddef.h>

/* Undefined/Incomplete Types (TYPE_UNDEFINED) */
struct undefined_struct;      /* Forward declaration, never defined */
union undefined_union;        /* Forward declaration, never defined */

/* Scalar Types (TYPE_SCALAR) */
typedef int scalar_int;
typedef char scalar_char;
typedef short scalar_short;
typedef long scalar_long;
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;
typedef _Complex float scalar_complex_float;
typedef _Complex double scalar_complex_double;
typedef __int128 scalar_int128;  /* GNU extension */

/* String Types (TYPE_STRING) */
typedef char* string_ptr;
typedef const char* const_string_ptr;

/* Struct Types (TYPE_STRUCT) */
struct simple_struct {
    int a;
    char b;
    float c;
    double d;
};

struct nested_struct {
    struct simple_struct inner;
    long extra;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int : 24;  /* Padding */
};

struct packed_struct {
    char a;
    int b;
    short c;
} __attribute__((packed));

struct aligned_struct {
    double data;
    int tag;
} __attribute__((aligned(64)));

/* Union Types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    char as_char[4];
};

union tagged_union {
    enum { INT_TAG, FLOAT_TAG, STRING_TAG } tag;
    struct {
        int type;
        union {
            int int_val;
            float float_val;
            char* string_val;
        } data;
    } value;
};

/* Pointer Types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef struct simple_struct* struct_ptr;
typedef void (*void_func_ptr)(void);

/* Array Types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef char char_array_2d[5][10];
typedef float float_array_3d[3][4][5];

/* Callback Types (TYPE_CALLBACK) */
typedef int (*int_callback)(int);
typedef void (*void_callback)(void);
typedef char* (*string_callback)(int, const char*);
typedef int (*variadic_callback)(int, ...);

/* Complex nested type with all categories */
struct master_struct {
    /* Scalar members */
    int id;
    float score;
    double precision;
    _Bool active;
    
    /* String member */
    char* name;
    
    /* Struct member */
    struct nested_struct data;
    
    /* Union member */
    union simple_union variant;
    
    /* Pointer members */
    void* opaque;
    struct master_struct* next;
    struct master_struct** prev;
    
    /* Array members */
    int counts[20];
    float matrix[3][3];
    
    /* Callback member */
    int (*processor)(struct master_struct*, int);
    
    /* Anonymous union */
    union {
        long as_long;
        double as_double;
    } anonymous;
    
    /* Bitfield */
    unsigned int flags : 8;
    
    /* Flexible array member */
    int flexible_array[];
} __attribute__((aligned(32)));

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function declarations using these types */
void process_struct(struct simple_struct* s);
int calculate(int_callback cb, int value);
union simple_union create_union(void);
struct master_struct* create_linked_list(int count);

/* External references to force type usage */
extern volatile int external_counter;
extern void external_function(void* data);

#endif /* TYPES_H */
