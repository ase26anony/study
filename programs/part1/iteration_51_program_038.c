/* Primary header file for gengtype parsing coverage test */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses */
typedef int (*complex_func_ptr)(int (*inner)(char[10]), 
                                struct {int a; int b;},
                                void (*)(int, int));

/* GTY annotation with nested balanced tokens */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* Multi-dimensional array with parenthesized size expression */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Nested union with struct containing arrays */
typedef union {
    struct {
        int x;
        char arr[5];
    };
    long l;
    double d[(2+3)];
} nested_union_t GTY((tag("UNION")));

/* Function pointer with attributes containing balanced parentheses */
typedef void (*__attribute__((stdcall, aligned(8))) api_function)(
    int __attribute__((aligned(16))),
    char *__attribute__((nonnull(1, 2)))
);

/* Complex struct with GTY chain operations */
GTY((chain_next = "next", chain_prev = "prev")) 
struct linked_list {
    struct linked_list *next;
    struct linked_list *prev;
    int data[(10+2)];
    char buffer[sizeof(struct {int a; double b;})];
};

/* Typedef with GCC attributes containing nested parentheses */
typedef int __attribute__((aligned(16), 
                          packed, 
                          deprecated("Use int32_t instead"))) 
        aligned_int;

/* Pointer to array of function pointers */
typedef int (*(*complex_array_ptr)[5])(char *, 
                                       struct {int x; int y;});

/* Include another header with more complex types */
#include "test_attributes.h"

#endif /* TEST_GTY_H */
