/* Primary test header for gengtype parser coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Test 1: Complex function pointer with nested parameter lists */
typedef int (*complex_fp)(int (*inner)(char[10]), 
                         struct {int a; int b;});

/* Test 2: GTY annotation with nested tokens */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* Test 3: Multi-dimensional array with parenthesized size expression */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Test 4: Nested union/struct with arrays */
typedef union {
    struct {
        int x;
        char arr[5];
    };
    long l;
} nested_union_t;

/* Test 5: GTY chain with array containing expression */
GTY((chain_next, chain_prev)) struct list {
    struct list *next;
    int data[(10+2)];
};

/* Test 6: Function pointer returning pointer to array */
typedef int (*(*fp_ret_array)[5])(void);

/* Test 7: Nested GTY annotations with complex types */
GTY(()) struct outer {
    GTY((skip)) struct inner {
        int * GTY((tag("1"))) ptr;
        char buffer[100];
    } nested;
    int count;
};

/* Test 8: Attribute specifications with balanced parentheses */
typedef int __attribute__((aligned(16), packed)) aligned_int;
typedef void (*__attribute__((stdcall)) api_fn)(int);

/* Test 9: Pointer to function returning pointer to function */
typedef void (*(*complex_callback)(int, char**))(double);

/* Test 10: Array of pointers to functions with nested parameter */
typedef void (*signal_handlers[32])(int, struct __attribute__((packed)) {
    int sig;
    void *context;
});

#endif /* TEST_GTY_H */
