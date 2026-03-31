#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses for function pointers */
typedef int (*complex_fp1)(int (*inner)(char[10]), 
                          struct {int a; int b;});

/* Multi-dimensional array with parenthesized size expressions */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Struct within typedef containing arrays and nested structs */
typedef union {
    struct {
        int x;
        char arr[5];
        struct {
            double d;
            float f[2];
        } inner;
    };
    long l;
} nested_union_t GTY((user));

/* GTY annotation with skip and nested tokens */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* Chain structure with array in data field */
struct GTY((chain_next = "next", chain_prev = "prev")) list {
    struct list *next;
    struct list *prev;
    int data[(10+2)];
    char name[20][(sizeof(int)*2)];
};

/* Function pointer type with multiple parameter lists */
typedef void (*(*nested_func_ptr)(int, char))(double, float[3]);

/* Complex array of function pointers */
typedef int (*(*array_of_fp[5])(void))[10];

/* Include another header with more complexity */
#include "test_attributes.h"

#endif /* TEST_GTY_H */
