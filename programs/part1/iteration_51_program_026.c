/* Primary test header for gengtype parser coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses for function pointers */
typedef int (*complex_func_ptr)(int (*inner)(char[10]), 
                                struct {int a; int b;},
                                void (*callback)(int, int));

/* Multi-dimensional array with parenthesized size expressions */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20][(2 + 3) * 4];

/* Struct within typedef containing arrays and nested structs */
typedef struct GTY((user)) {
    int data[5][(10)];
    struct {
        char name[20];
        int scores[(3 + 2)];
    } GTY((tag("inner_struct"))) info;
    void (*operations[3])(int, char);
} complex_struct_t;

/* Union with anonymous struct containing arrays */
typedef union GTY((chain_next("next"), chain_prev("prev"))) {
    struct {
        int x;
        char arr[5][10];
        long matrix[3][(4)];
    };
    long l;
    double d[(2 * 3)];
} nested_union_t;

/* Function pointer with GCC attributes containing balanced parentheses */
typedef void (__attribute__((stdcall, aligned(16))) *api_function)(
    int __attribute__((aligned(8))),
    char *__attribute__((nonnull(1, 2)))
);

/* Nested pointer types with GTY markers in complex positions */
typedef GTY((user)) struct node * GTY((skip)) node_ptr_array[10];

/* Array of function pointers */
typedef int (*(*func_ptr_array[5]))(int, char (*)[10]);

/* Include the secondary header for more complexity */
#include "test_complex.h"

#endif /* TEST_GTY_H */
