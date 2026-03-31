#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses */
typedef int (*complex_func_ptr)(
    int (*inner_callback)(char[10]), 
    struct {int a; int b;} param
);

/* GTY annotation with nested balanced tokens */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* Multi-dimensional array with parenthesized size expression */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Struct within typedef containing arrays */
typedef union {
    struct {
        int x;
        char arr[5];
    };
    long l;
} nested_union_t GTY((tag("UNION")));

/* Function pointer with attribute containing balanced parentheses */
typedef void (*__attribute__((stdcall, aligned(8))) api_function)(
    int __attribute__((aligned(16))) param
);

/* Chain structure with array in GTY annotation */
struct GTY((chain_next = "next", chain_prev = "prev")) linked_item {
    struct linked_item *next;
    struct linked_item *prev;
    int data[(10+2)];
};

/* Nested struct with bit-fields and arrays */
typedef struct GTY(()) container {
    struct {
        unsigned int flags:8;
        unsigned int count:24;
    } header;
    char buffer[256];
    struct container *children[4];
} container_t;

/* Complex pointer to array of function pointers */
typedef int (*(*complex_array_ptr)[10])(
    char (*callback)(int[3][3])
);

#endif /* TEST_GTY_H */
