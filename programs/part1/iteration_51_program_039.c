/* Primary header file for gengtype parser coverage testing */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses for function pointers */
typedef int (*complex_func_ptr)(
    int (*inner_callback)(char[10], 
        struct {int a; int b;}),
    void (*__attribute__((noreturn)) error_handler)(int)
);

/* GTY annotation with nested balanced tokens in arguments */
typedef GTY((user, 
    (ptr_alias("node_ptr")),
    (desc("tag:1")))
) struct tree_node * GTY((skip)) tree_node_ptr;

/* Multi-dimensional array with parenthesized size expressions */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20][
    (__alignof__(double) * 2)
];

/* Nested struct/union within typedef with GTY marker */
typedef union GTY((tag("UNION_TYPE"))) {
    struct GTY((desc("1"))) {
        int x;
        char arr[5][(2+3)];
        struct {
            long bits:8;
            long more:((16)/2);
        } GTY((skip)) packed;
    } inner;
    long l;
    double d[(sizeof(int) + 2)];
} nested_union_t;

/* Function pointer type with attribute containing balanced parentheses */
typedef void (__attribute__((format(printf, 1, 2))) 
    *log_function)(const char *fmt, ...);

/* Include secondary header with more complex types */
#include "test_attributes.h"

#endif /* TEST_GTY_H */
