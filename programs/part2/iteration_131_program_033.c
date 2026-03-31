/* Test header with complex GTY annotations to trigger consume_balanced logic */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested delimiters */
GTY((tag("complex_struct"))) struct test_struct {
    /* Multiple levels of parentheses and brackets */
    int (* volatile arr[10])[5];
    void (*callback_array[3])(int (*)(char *));
};

/* Nested union/struct with bit-fields and multi-dimensional arrays */
GTY((desc("%1"), var)) union inner_container {
    struct {
        short f:3;
        int g[2][2];
        long double (*matrix_ptr)[4][4];
    } nested;
    long h;
    char (*string_array[5])[20];
};

/* Function pointer type with deeply nested parameter lists */
typedef GTY((chain_next = "%h.next", chain_prev = "%h.prev")) 
struct linked_node {
    /* Complex function pointer returning pointer to array of function pointers */
    int (*(*complex_callback)(int, char *))[5];
    struct linked_node *next;
    struct linked_node *prev;
} node_t;

/* Type with balanced brackets in GTY arguments */
GTY((user("matrix_type"), param1 = "int (*)(int[10][20])")) 
typedef int multi_matrix[4][4][4];

/* Edge case: character that might trigger default case */
GTY((var)) int normal_type;
/* The following line contains '@' which might trigger default/advance() */
GTY((var)) int weird@type;  /* This should trigger default case */

#endif /* GT_TEST1_H */
