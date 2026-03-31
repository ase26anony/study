/* Primary header file for gengtype parser coverage testing */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses (function pointers) */
typedef int (*complex_fp1)(int (*inner)(char[10]), 
                          struct {int a; int b;});

/* GTY annotation with nested tokens */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* Multi-dimensional array with parenthesized size expression */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Struct definition within typedef containing arrays */
typedef union {
    struct {
        int x;
        char arr[5];
    };
    long l;
} nested_union_t;

/* GTY chain with array containing expression in brackets */
GTY((chain_next, chain_prev)) struct list {
    struct list *next;
    int data[(10+2)];
};

/* Nested function pointer type with multiple parameter levels */
typedef void (*(*nested_func_ptr)(int, char))(double, 
    struct { int x; int y[(3+2)]; });

/* Include complex types from secondary header */
#include "test_attributes.h"

#endif /* TEST_GTY_H */
