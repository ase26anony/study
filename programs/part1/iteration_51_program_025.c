/* Primary test header for gengtype parser coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses */
typedef int (*complex_func_ptr)(
    int (*inner_callback)(char[10], int),
    struct {
        int a;
        double b[5];
    },
    void (*__attribute__((noreturn)) error_handler)(void)
);

/* GTY annotation with nested balanced tokens */
typedef GTY((user, 
    ptr_alias("node_ptr"),
    desc("tag::0")
)) struct node * GTY((skip)) node_ptr_array[10][(sizeof(void*) == 8) ? 20 : 10];

/* Multi-dimensional array with parenthesized size expressions */
typedef int matrix_t[
    5][
    (sizeof(long) > 4) ? 10 : 20
][
    __alignof__(double)
];

/* Nested struct/union within typedef with GTY */
typedef union GTY((tag("UNION"))) {
    struct GTY((desc("%1.a"))) {
        int x;
        char arr[5][10];
        long bitfield: 8;
    } inner;
    long l;
    double d[3][(2 + 3)];
} nested_union_t;

/* Function pointer type with attribute containing balanced parentheses */
typedef void (__attribute__((format(printf, 1, 2), nonnull(1))) 
    *log_func_t)(const char *fmt, ...);

/* Complex type with all three delimiters deeply nested */
typedef struct GTY(()) {
    int (*compare)(
        const void *a,
        const void *b,
        int options[3][(MAX_DEPTH + 1)]
    );
    union {
        char data[100];
        struct {
            int length;
            char *buffer;
        } dynamic;
    } content;
} complex_container_t;

/* Include another header with more complexity */
#include "test_attributes.h"

#endif /* TEST_GTY_H */
