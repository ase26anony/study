/* Primary test header for gengtype parsing coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses for function pointers */
typedef int (*complex_func_ptr_t)(int (*inner)(char[10]), 
                                  struct {int a; int b;},
                                  void (*callback)(int, int));

/* Multi-dimensional array with parenthesized size expressions */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20][(2+3)*4];

/* Struct within typedef containing arrays and nested structs */
typedef struct GTY((user)) {
    int data[(10+2)];
    struct GTY((tag("inner_tag"))) {
        char buffer[256];
        int * GTY((skip)) ptr_array[5];
    } inner;
    union {
        long l;
        double d;
        char str[50];
    } value;
} complex_struct_t;

/* Function pointer with GCC attributes containing balanced parentheses */
typedef void (* __attribute__((aligned(16), 
                               deprecated("Use new_api instead"))) 
              api_func_t)(int __attribute__((nonnull(1,2))) param);

/* Nested union with bit-fields and arrays */
typedef union GTY((chain_next("next"), chain_prev("prev"))) {
    struct {
        unsigned int flags:8;
        unsigned int :24;  /* Padding */
        int arr[3][2];
    } bits;
    long long as_long;
    struct nested * GTY((skip)) next;
} chainable_union_t;

/* Extremely complex type mixing all delimiters */
typedef int (*(*nested_fp_array[5])(struct { 
    int x; 
    char name[50]; 
}))(int, 
    char (*)[10], 
    void (*)(void));

/* Include macro-based complex types */
#include "test_macro_expansion.h"

#endif /* TEST_GTY_H */
