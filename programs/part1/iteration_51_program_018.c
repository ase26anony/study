/* Primary test header for gengtype parser coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses */
typedef int (*complex_func_ptr)(
    int (*inner_callback)(char[10][(sizeof(double) > 4) ? 8 : 4]),
    struct {
        int a;
        long b[(2 + 3) * 4];
    } __attribute__((aligned(16)))
);

/* GTY annotation with nested balanced tokens */
typedef GTY((user, 
            (ptr_alias("node_ptr")),
            (desc("tag @1"))
           )) 
       struct node {
    struct node * GTY((skip)) next;
    int data[5][(sizeof(void*) == 8) ? 10 : 5];
    union {
        char buffer[100];
        struct {
            int flags;
            long values[3];
        } __attribute__((packed));
    } payload;
} * GTY((tag("1"))) node_ptr;

/* Multi-dimensional array with parenthesized expressions */
typedef int matrix_type[
    5][
    (sizeof(long long) > 8) ? 12 : 6
    ][
    (1 << 3) + 2
];

/* Nested struct within union within typedef */
typedef union {
    struct {
        int x;
        char arr[5][(10 % 3) + 2];
    } inner;
    struct {
        long l;
        short s[((4 * 2) - 1)];
    } __attribute__((aligned(8)));
    double d;
} nested_union_t GTY(());

/* Function pointer with complex parameter list */
typedef void (*api_function)(
    int param1,
    char *param2[],
    struct {
        int count;
        int items[10];
    } *config,
    void (*callback)(
        int status,
        const char *message[(20 / 2)]
    )
) __attribute__((stdcall));

/* Include secondary header with more complex types */
#include "test_attributes.h"

#endif /* TEST_GTY_H */
