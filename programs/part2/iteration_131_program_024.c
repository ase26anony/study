/* Test header with complex GTY annotations to trigger consume_balanced calls */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested delimiters */
GTY((tag("complex_struct"))) struct test_struct {
    /* Multiple levels of parentheses and brackets */
    int (* volatile arr[10])[5];
    void (*func_ptr_array[3])(int (*)(char [][10]));
};

/* Nested union/struct with bit-fields and multi-dimensional arrays */
GTY((var, desc("%1"))) union inner_container {
    struct {
        short field1:3;
        int field2[2][2];
        long (*matrix_func)(double (*)[5][5]);
    } nested;
    long long_value;
    /* Function pointer with complex parameter */
    void (*callback)(int (*(*callback_array[5])(void))[10]);
};

/* Chain structure with function pointer types containing parameter lists */
typedef GTY((chain_next = "%h.next", chain_prev = "%h.prev")) 
struct node {
    /* Deeply nested function pointer type */
    int (*complex_callback)(int (*(*nested_array[5])(void)), 
                           struct { int x; double y[2]; } param);
    struct node * GTY((skip)) next;
    struct node *prev;
} node_t;

/* Type definition with balanced brackets in GTY arguments */
GTY((user("matrix_type"), param("int (*indexer)(int[10][10])"))) 
typedef int complex_matrix[4][4];

#endif /* GT_TEST1_H */
