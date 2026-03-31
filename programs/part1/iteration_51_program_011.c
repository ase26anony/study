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
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20][(2 + 3)];

/* GTY annotation with nested balanced tokens in arguments */
typedef GTY((user, 
            (ptr_alias("node_ptr")), 
            (desc("node_@1.depth")))
          ) struct node * GTY((skip)) node_ptr;

/* Struct with GTY chain annotations and nested arrays */
GTY((chain_next = "next", chain_prev = "prev")) 
struct complex_list {
    struct complex_list *next;
    struct complex_list *prev;
    int data[(10+2)][5];
    char * GTY((tag("0"))) name;
};

/* Union containing struct with arrays */
typedef union {
    struct {
        int x;
        char arr[5][(3*2)];
    } inner;
    long l;
    double matrix[2][(sizeof(int)*2)];
} nested_union_t GTY((user));

/* Function pointer type with multiple nested parameter lists */
typedef void (*nested_fp)(
    int, 
    char *(*)(int[5], struct {int x;}), 
    union { 
        int a; 
        double b[(2+3)]; 
    } param
);

/* GCC attributes with balanced parentheses */
typedef int __attribute__((aligned(16), 
                          packed, 
                          deprecated("use new_type instead"))) 
    aligned_int_type;

/* Function pointer with stdcall attribute */
typedef void (*__attribute__((stdcall, 
                             noinline)) 
    api_function)(int param1, char *param2[(5)]);

/* Include secondary complex type */
#include "test_secondary.h"

#endif /* TEST_GTY_H */
