#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested delimiters */
GTY((tag("complex_struct"))) struct test_struct {
    /* Multiple levels of parentheses and brackets */
    int (* volatile arr[10])[5];
    void (*func_ptr_array[3])(int (*)[2][2]);
};

/* Nested union/struct with bit-fields and multi-dimensional arrays */
GTY((var, desc("%1"))) union inner_container {
    struct {
        short field1:3;
        int field2[2][2];
        long (*matrix_ptr)[4][4];
    } nested;
    long long_value;
    void (*callback)(int (*(*array_of_func_ptrs)[5])(void));
};

/* Function pointer type with deeply nested parameter lists */
typedef GTY((chain_next = "%h.next", chain_prev = "%h.prev")) 
struct linked_node {
    /* Complex function pointer signature */
    int (*processor)(int (*(*input_handler)[5])(char (*)[10]), 
                     void (*output_callback)(int[][3]));
    struct linked_node *next;
    struct linked_node *prev;
} node_t;

/* Edge case: unusual character to potentially trigger default case */
GTY((user)) typedef int matrix_type[4][4];

/* Another edge case with @ symbol in comment - might affect parsing */
GTY((var)) extern int (*complex_decl)(); /* weird@comment */

#endif /* GT_TEST1_H */
