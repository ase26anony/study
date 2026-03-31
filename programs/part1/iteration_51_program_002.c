/* Primary header file for gengtype parser coverage testing */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses for function pointers */
typedef int (*complex_fp1)(int (*inner)(char[10]), 
                          struct {int a; int b;});

/* Multi-dimensional array with parenthesized size expression */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Struct within typedef containing arrays */
typedef union {
    struct {
        int x;
        char arr[5];
    };
    long l;
} nested_union_t;

/* GTY annotation with nested balanced tokens */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* Chain structure with array in GTY annotation */
GTY((chain_next = "next", chain_prev = "prev")) 
struct list {
    struct list *next;
    struct list *prev;
    int data[(10+2)];
};

/* Complex function pointer type with GTY */
typedef GTY(()) void (*signal_handler)(int, 
    GTY(()) void (*callback)(char buffer[256]));

/* Nested struct with bit-fields and arrays */
typedef struct GTY(()) {
    struct {
        unsigned int flags:8;
        char name[32];
    } header;
    union {
        int values[10];
        struct {
            long ptrs[5];
        };
    } data;
} container_t;

/* GCC attributes with balanced parentheses */
typedef int __attribute__((aligned(16), packed)) aligned_int;

/* Function pointer with stdcall attribute */
typedef void (*__attribute__((stdcall)) api_fn)(int param[(sizeof(int)*2)]);

/* Multiple levels of nesting */
typedef char *(*(*nested_fp_arr[5])(int))(float, 
    struct { char buf[100]; });

#endif /* TEST_GTY_H */
