/* Primary header file for gengtype parser coverage testing */
#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "test_nested.h"
#include "test_macros.h"

/* Complex typedef with deeply nested parentheses */
typedef int (*complex_func_ptr)(
    int (*inner_callback)(char[10][(sizeof(double) > 8) ? 5 : 10]),
    struct {
        int a;
        long b[(2+3)*4];
    } __attribute__((aligned(16), packed))
);

/* GTY annotation with nested balanced tokens */
typedef GTY((user, 
            (ptr_alias("node_ptr")),
            (desc("tag @1"),
             tag("1:root, 2:leaf"))
           )) 
struct tree_node {
    struct tree_node * GTY((skip)) left;
    struct tree_node * GTY((skip)) right;
    int data[5][(sizeof(void*) == 8) ? 10 : 5];
    char name[(MAX_NAME_LEN + 2)];
} * GTY((tag("1"))) tree_node_t;

/* Nested union with struct containing arrays */
typedef union {
    struct {
        int x;
        char arr[5][10];
        struct {
            short s;
            long l[3];
        } inner;
    };
    long values[((10/2)+3)];
    void (*callback)(int, char **);
} __attribute__((packed)) nested_union_t GTY(());

/* Function pointer type with multiple nested parameter lists */
typedef void (*(*signal_handler_registrar)(
    void (*handler)(int, siginfo_t *, void *),
    struct {
        int flags;
        void *extra_data[(MAX_HANDLERS)];
    }
))(
    int signum,
    const struct sigaction * GTY((skip)) oldact
);

/* Multi-dimensional array with parenthesized size expressions */
typedef int matrix_type[
    5
][
    (sizeof(long long) > 8) ? 10 : 20
][
    (1 << 3) + 2
] GTY(());

/* GTY chain with nested brackets */
GTY((chain_next = "next", chain_prev = "prev"))
struct linked_list {
    struct linked_list *next;
    struct linked_list *prev;
    int items[LIST_SIZE][(ITEMS_PER_ROW)];
    union {
        char *str;
        int num;
        struct {
            float x, y;
            double coords[3][2];
        } point;
    } data;
};

/* Complex attribute specification */
typedef int __attribute__((
    aligned(32),
    vector_size(16),
    deprecated("Use vector_int instead")
)) vector_int_t GTY(());

/* Nested function pointer in struct */
typedef struct {
    int (*compare)(
        const void *,
        const void *,
        void *user_data[(USER_DATA_SIZE)]
    );
    void (*cleanup)(
        struct {
            int ref_count;
            char *buffer[BUFFER_COUNT];
        } *
    );
} comparator_set_t GTY((user));

#endif /* TEST_GTY_H */
