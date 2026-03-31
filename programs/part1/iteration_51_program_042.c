#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex function pointer with nested parameter lists */
typedef int (*complex_fp)(int (*inner)(char[10]), struct {int a; int b;});

/* Multi-dimensional array with parenthesized size expressions */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Struct within typedef containing arrays and bit-fields */
typedef union { 
    struct { 
        int x; 
        char arr[5]; 
        unsigned int bitfield : 4;
    }; 
    long l; 
} nested_union_t GTY((user));

/* GTY annotation with skip marker on complex type */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* Chain structure with array in data field */
GTY((chain_next, chain_prev)) struct list { 
    struct list *next; 
    struct list *prev;
    int data[(10+2)]; 
};

/* Nested function pointer type */
typedef void (*(*nested_func_ptr)(int))(char, double);

/* Array of function pointers */
typedef int (*func_array_t[5])(void);

/* Complex type with multiple balanced groups */
typedef struct {
    int (*compare)(const void *, const void *);
    char buffer[256];
} sorter_t GTY(());

/* GCC attributes with balanced parentheses */
typedef int __attribute__((aligned(16), packed)) aligned_int;
typedef void (*__attribute__((stdcall)) api_fn)(int);

/* Pointer to array */
typedef int (*ptr_to_array)[10];

/* Reference to test_macros.h expansions */
typedef GTY_USER_ARGS my_struct { 
    int i; 
    PTR_TO(NESTED_ARRAY) data;
} my_struct_t;

/* Another complex GTY annotation */
GTY((desc("%1.var"), tag("COMPLEX_TYPE"))) struct complex_type {
    int var;
    struct complex_type *next GTY((skip));
};

#endif /* TEST_GTY_H */
