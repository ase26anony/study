/* Primary test header for gengtype coverage testing */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses for function pointers */
typedef int (*complex_fp1)(int (*inner)(char[10]), 
                           struct {int a; int b;});

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
} nested_union_t;

/* GTY chain with array containing expression in brackets */
GTY((chain_next, chain_prev)) struct list { 
    struct list *next; 
    int data[(10+2)]; 
};

/* Function pointer with multiple nested parameter lists */
typedef void (*(*nested_func_ptr)(int, double))(char *, int[][5]);

/* Struct with bit-fields and nested arrays */
typedef struct {
    unsigned int flags:8;
    char name[20];
    int matrix[3][(2+3)];
} complex_struct_t;

/* GCC attributes with balanced parentheses */
typedef int __attribute__((aligned(16), packed)) aligned_int;

/* Function pointer with stdcall attribute */
typedef void (*__attribute__((stdcall)) api_fn)(int);

/* Deeply nested: array of pointers to functions returning structs */
typedef struct result {
    int status;
    char message[50];
} (*operation_array_t[10])(int param, char buffer[]);

/* Include more complex types from secondary headers */
#include "test_attributes.h"

#endif /* TEST_GTY_H */
