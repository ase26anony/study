/* Primary test header for gengtype parser coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses */
typedef int (*complex_func_ptr)(
    int (*inner_callback)(char[10]), 
    struct {int a; int b;} param
);

/* Multi-dimensional array with parenthesized size expression */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Nested union with struct containing arrays */
typedef union {
    struct {
        int x;
        char arr[5];
    } inner;
    long l;
    double d[(2+3)];
} nested_union_t GTY((user));

/* GTY annotation with nested tokens */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* Chain structure with array in data */
struct GTY((chain_next = "next", chain_prev = "prev")) list {
    struct list *next;
    struct list *prev;
    int data[(10+2)];
    char buffer[5][(sizeof(int)*2)];
};

/* Function pointer type with GTY markers */
typedef GTY((ptr_alias("func_ptr_alias"))) 
    void (*signal_handler)(
        int sig,
        void (*cleanup)(void *data[5]),
        struct { int code; char msg[20]; } *context
    );

/* Nested type with GCC attributes */
typedef int __attribute__((aligned(16), packed)) aligned_int;

/* API function pointer with stdcall attribute */
typedef void (*__attribute__((stdcall)) api_fn)(
    int param1,
    char param2[(sizeof(double) + 2)]
);

/* Complex nested structure with bitfields */
typedef struct GTY(()) {
    unsigned int flags:4;
    unsigned int :4;  /* unnamed bitfield */
    int array[3][(1 << 2)];
    union {
        long l;
        struct {
            short s;
            char c[8];
        } inner;
    } data;
} bitfield_struct_t;

#endif /* TEST_GTY_H */
