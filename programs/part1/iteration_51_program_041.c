/* Primary test header for gengtype parsing coverage */
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
    void (*func_ptr)(int, char);
} nested_union_t;

/* GTY annotation with nested balanced tokens */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* Chain structure with array containing expression */
GTY((chain_next = "next", chain_prev = "prev")) 
struct list {
    struct list *next;
    struct list *prev;
    int data[(10+2)];
    char buffer[sizeof(struct list*) * 2];
};

/* Function pointer type with multiple nested parameter lists */
typedef void (*(*nested_func_ptr)(int (*(*)(char[5]))(double)))(float);

/* Array of function pointers */
typedef int (*fp_array_t[5])(char (*)[10], struct {int x;});

/* Deeply nested with all three bracket types */
typedef struct {
    int (*compute)(int matrix[3][(2+3)], 
                   void (*callback)(struct {char c; int i;}));
    union {
        char str[10];
        int *ptr_array[(sizeof(int) + 2)];
    } data;
} complex_struct_t;

/* Include GCC attributes with balanced parentheses */
typedef int __attribute__((aligned(16), packed)) aligned_int;

/* Function pointer with attributes */
typedef void (*__attribute__((stdcall)) api_fn)(int, 
    __attribute__((nonnull)) char *);

/* Nested typedef with GTY markers */
GTY(()) typedef struct GTY((tag("NODE"))) tree_node {
    struct tree_node *GTY((skip)) left;
    struct tree_node *right;
    int value;
    char name[(MAX_NAME_LEN + 1)];
} tree_node_t;

/* Macro-based complex type */
typedef GTY_USER_ARGS my_struct {
    int i;
    PTR_TO(NESTED_ARRAY) matrix_ptr;
    COMPLEX_FP_TYPE handler;
} my_struct_t;

/* Array type with nested dimensions */
typedef int three_d_array[(2+3)][sizeof(double)][10];

/* Bitfield struct within typedef */
typedef struct {
    unsigned int flag1:1;
    unsigned int flag2:2;
    unsigned int :3;  /* Padding */
    unsigned int value:8;
    int array[((8 + 7) / 8)]; /* Array size based on bitfield */
} bitfield_struct_t;

#endif /* TEST_GTY_H */
