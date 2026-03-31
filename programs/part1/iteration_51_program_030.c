/* Primary header file for gengtype parsing coverage test */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses for function pointers */
typedef int (*complex_fp1)(int (*inner)(char[10]), 
                          struct {int a; int b;});

/* Multi-dimensional array with parenthesized size expression */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Struct within typedef containing arrays and nested braces */
typedef union { 
    struct { 
        int x; 
        char arr[5]; 
    }; 
    long l; 
} nested_union_t;

/* GTY annotation with nested tokens */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* GTY with chain_next and array with expression in brackets */
GTY((chain_next = "next", chain_prev = "prev")) 
struct list { 
    struct list *next;
    struct list *prev;
    int data[(10+2)];
};

/* Complex function pointer type with GTY */
typedef GTY(()) void (*signal_handler)(int, 
                                       GTY(()) void *user_data,
                                       struct {int code; char msg[50];} *ctx);

/* Nested typedef with multiple balanced groups */
typedef int (*(*nested_fp_arr[3]))(char (*)[(sizeof(int)*2)], 
                                  union {int i; float f;});

/* Include another header with more complex types */
#include "test_attributes.h"

#endif /* TEST_GTY_H */
