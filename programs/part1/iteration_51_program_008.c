/* Primary header file for gengtype parser coverage test */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses for function pointers */
typedef int (*complex_fp)(int (*inner)(char[10]), struct {int a; int b;});

/* Multi-dimensional array with parenthesized size expression */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Struct within typedef containing arrays and nested structs */
typedef struct GTY((user)) {
    int data;
    char buffer[(10 + 2)];
    struct GTY((tag("inner_tag"))) {
        int x;
        int y[3][3];
    } inner;
} nested_struct_t;

/* Union with anonymous struct containing arrays */
typedef union GTY((chain_next("next"), chain_prev("prev"))) {
    struct {
        int x;
        char arr[5];
    };
    long l;
    double d[2][(sizeof(int) * 2)];
} nested_union_t;

/* Function pointer with GCC attributes containing balanced parentheses */
typedef void (*__attribute__((stdcall, aligned(8))) api_fn)(int, char);

/* Aligned typedef with attribute containing nested parentheses */
typedef int __attribute__((aligned(16), packed, mode(SI))) aligned_int;

/* Pointer to array of function pointers */
typedef int (*(*array_of_fp)[5])(void);

/* GTY annotation with skip and nested arguments */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* Complex nested type with multiple GTY markers */
typedef GTY((desc("%1.flag"), param_is(struct flag_info))) 
       struct GTY((tag("flag_node"))) flag_node {
    int flag;
    struct flag_node * GTY((skip)) next;
    void (* GTY((skip)) handler)(struct flag_node *);
} flag_node_t;

/* Include more complex types from secondary header */
#include "test_complex.h"

#endif /* TEST_GTY_H */
