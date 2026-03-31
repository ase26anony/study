/* Primary test header for gengtype parser coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses */
typedef int (*complex_func_ptr)(
    int (*inner_callback)(char[10][(sizeof(double) > 8) ? 5 : 10]),
    struct {
        int a;
        int b[(2+3)*4];
    } __attribute__((aligned(16)))
);

/* GTY annotation with nested token groups */
typedef GTY((user, 
            (ptr_alias("node_ptr")),
            (desc("tag @1"))
           )) 
struct node {
    struct node * GTY((skip)) next;
    int data[5][(sizeof(void*) == 8) ? 10 : 5];
    union {
        long l;
        char buf[20];
    } payload;
} *node_ptr_t;

/* Multi-dimensional array with parenthesized size expressions */
typedef int matrix_t[
    5][
    (sizeof(long long) > 8) ? 20 : 10
][
    __builtin_alignof(double)
];

/* Nested struct within union within typedef */
typedef union {
    struct {
        int x;
        char arr[5][10];
        struct {
            short s;
            int bits:4;
        } inner;
    } s;
    long l[3];
    void (*func)(int, char[(10+5)]);
} __attribute__((packed)) nested_union_t;

/* Function pointer type with complex parameter list */
typedef void (*(*signal_handler_factory)(
    int sig,
    void (*old_handler)(int, siginfo_t *, void *),
    struct {
        int flags;
        char name[32];
    } *config
))(int, void *);

/* GTY chain with nested arrays */
GTY((chain_next = "next", chain_prev = "prev"))
struct list {
    struct list *next;
    struct list *prev;
    int items[][(2*3)+1];
    struct {
        char * GTY((tag("0"))) name;
        int id;
    } metadata;
};

/* Using macros that expand to balanced groups */
typedef GTY_USER_ARGS my_struct_t;

/* Complex pointer type using macro expansion */
typedef PTR_TO(ARRAY_2D) complex_matrix_ptr;

/* Attribute with nested parentheses */
typedef int __attribute__((
    aligned(32),
    deprecated("Use int64_t instead"),
    vector_size(16)
)) vector_int_t;

/* Another deeply nested case */
typedef struct outer {
    union {
        int (*funcs[5])(char *args[(10)]);
        struct {
            long counter;
            char buffer[100];
        };
    } u;
    enum {
        STATE_A,
        STATE_B = (1 << 3)
    } state;
} outer_t GTY(());

#endif /* TEST_GTY_H */
