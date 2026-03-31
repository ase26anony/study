#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested delimiters */
GTY((tag("complex_struct"))) struct test_struct {
    /* Multiple levels of parentheses and brackets */
    int (* volatile arr[10])[5];
    void (*callback_array[3])(int (*)(int[][5]), char *);
};

/* Nested union/struct with bit-fields and multi-dimensional arrays */
GTY((desc("%0"), var)) union inner_container {
    struct {
        short f:3;
        int g[2][2];
        long double (*matrix_ptr)[4][4];
    } nested;
    long h;
    void (*func_ptr)(int (*(*callback_table[5])(void))[10]);
};

/* Function pointer type with deeply nested parameter lists */
typedef GTY((chain_next = "%h.next", chain_prev = "%h.prev")) 
struct linked_node {
    /* Complex function pointer returning array pointer */
    int (*(*get_matrix)(int size))[10][10];
    struct linked_node *next;
    struct linked_node *prev;
    /* Nested function pointer in array */
    void (*handlers[5])(int (*)(char (*)[20]), double);
} node_t;

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param1 = "int (*)(int[10][10])", 
     param2 = "void (*[5])(struct {int a; double b;})"))
typedef int complex_matrix[4][4][4];

#endif /* GT_TEST1_H */
