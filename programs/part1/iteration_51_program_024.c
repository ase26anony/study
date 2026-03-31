/* Primary header file for gengtype parser coverage testing */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses for function pointers */
typedef int (*complex_fp_type)(
    int (*inner_callback)(char[10]), 
    struct {int a; int b;} param
);

/* Multi-dimensional array with parenthesized size expressions */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Struct within typedef containing arrays and nested structs */
typedef struct GTY((user)) {
    int data;
    char buffer[(16 + 4)];
    struct GTY((tag("inner_tag"))) {
        float x;
        double y[2][2];
    } inner;
} nested_struct_t;

/* Union with anonymous struct containing arrays */
typedef union GTY((chain_next("next"), chain_prev("prev"))) {
    struct {
        int x;
        char arr[5];
    };
    long l;
    double matrix[3][(8/2)];
} nested_union_t;

/* Function pointer with GCC attributes containing balanced parentheses */
typedef void (*__attribute__((stdcall, aligned(16))) api_function)(
    int param1,
    char *__attribute__((nonnull(1, 2))) param2[]
);

/* Pointer to array of function pointers */
typedef int (*(*complex_array_ptr)[(10)])(
    struct GTY(()) { int a; char b[4]; } *arg
);

/* Nested GTY annotations with skip parameter */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* Typedef with multiple levels of pointer nesting */
typedef struct GTY((ptr_alias("tree_ptr"))) tree_node {
    struct tree_node *left;
    struct tree_node *right;
    int values[((3 * 2) + 1)];
} *tree_ptr, **tree_double_ptr;

#endif /* TEST_GTY_H */
