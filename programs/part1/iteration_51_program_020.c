/* Primary test header for gengtype parser coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex function pointer with nested parameter lists */
typedef int (*complex_fp_type)(int (*inner)(char[10]), 
                               struct {int a; int b;});

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

/* GTY annotation with skip marker on complex pointer type */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* Chain-linked structure with array in data field */
struct GTY((chain_next = "next", chain_prev = "prev")) list {
    struct list *next;
    struct list *prev;
    int data[(10+2)];
};

/* Function pointer with attributes containing balanced parentheses */
typedef void (*__attribute__((stdcall, aligned(8))) api_fn_type)(int, char);

/* Aligned type with multiple attributes */
typedef int __attribute__((aligned(16), packed, may_alias)) aligned_int_t;

/* Nested function pointer returning function pointer */
typedef int (*(*nested_fp_return)(void))(char *);

/* Array of function pointers */
typedef int (*fp_array_t[5])(int (*)(char), void *);

/* Deeply nested combinations */
typedef struct {
    int (*callback)(int matrix[3][(2+3)], 
                    union { 
                        int i; 
                        struct { char c; float f; } s; 
                    } u);
    void *ptr_array[((sizeof(void*) == 8) ? 10 : 5)];
} container_t GTY(());

#endif /* TEST_GTY_H */
