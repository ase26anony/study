/* Primary test header for gengtype parser coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested balanced tokens */
typedef int (*complex_func_ptr)(
    int (*inner)(char[10]), 
    struct {int a; int b;},
    union { 
        long l; 
        double d; 
        struct { 
            short s; 
            char c[5]; 
        } nested;
    }
);

/* GTY annotation with nested balanced tokens */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* Multi-dimensional array with parenthesized size expression */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Struct with nested arrays and bit-fields */
typedef GTY((chain_next, chain_prev)) struct list {
    struct list *next;
    struct list *prev;
    int data[(10+2)];
    unsigned flags:4;
    char name[20][(sizeof(int)*2)];
} list_t;

/* Union containing struct with array */
typedef union {
    struct {
        int x;
        char arr[5];
    } s;
    long l;
    double matrix[3][3];
} nested_union_t GTY((tag("UNION_TYPE")));

/* Function pointer type with GCC attributes */
typedef void (*__attribute__((stdcall, noreturn)) 
           api_function)(int, ...);

/* Aligned type with attribute containing balanced parens */
typedef int __attribute__((aligned(16), packed, 
           deprecated("use int32_t instead"))) aligned_int;

/* Nested pointer types */
typedef GTY(()) int (*(*nested_fp[5]))(void);

/* Include secondary header for more complexity */
#include "test_complex.h"

#endif /* TEST_GTY_H */
