#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structures */
GTY((tag("TEST_STRUCT"))) struct test_struct {
    /* Nested parentheses and brackets */
    int (* volatile arr[10])[5];
    void (*complex_func_ptr)(int (*callback)(int, char **));
};

/* Nested unions/structs with bit-fields and arrays */
GTY((var)) union inner_union {
    struct {
        short f:3;
        int g[2][2];
        long (*h_array[3])(void);
    };
    long h;
    struct {
        int (*nested[2][2])(char (*str_array[])[10]);
    } deeper;
};

/* Function pointer types with complex parameter lists */
typedef GTY((chain_next = "%h.next", chain_prev = "%h.prev")) 
struct node {
    /* Multiple levels of nested delimiters */
    int (*callback)(int (*(*array_ptr)[5])(void));
    void (*another_cb)(char (*(* *triple_ptr))(int[][3]));
    struct node *next;
    struct node *prev;
} node_t;

/* Type with deeply nested brackets */
GTY((user)) typedef int matrix_type[4][4];
GTY((length("%0 * 2"))) int (*deeply_nested_array)[2][3][4];

#endif /* GT_TEST1_H */
