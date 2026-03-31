/* Test header 1 - Complex nested delimiters in GTY annotations */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested parentheses */
GTY((tag("complex_array"))) struct test_struct {
    /* Multiple levels of pointer/array nesting */
    int (* volatile arr_ptr[10])[5];
    void (**func_table)(int, char);
};

/* Nested union/struct with bit-fields and multi-dimensional arrays */
GTY((var, desc("%1"))) union inner_container {
    struct {
        short flags:3;
        int matrix[2][2];
        long (*callback)(int (*handler)(void));
    } nested;
    long simple;
    char buffer[sizeof(struct { int x; double y; })];
};

/* Function pointer type with deeply nested parameter lists */
typedef GTY((chain_next = "%h.next", chain_prev = "%h.prev")) 
struct linked_node {
    /* Complex function pointer returning pointer to array of function pointers */
    int (*(*get_callbacks)(void))[5];
    struct linked_node *next;
    struct linked_node *prev;
} node_t;

/* Type definition with balanced brackets in GTY arguments */
GTY((user("matrix_type"), param("dimensions = [4][4]"))) 
typedef int complex_matrix[4][4];

/* Edge case: unusual character to potentially trigger default case */
GTY((var)) int weird@type;  /* '@' might trigger default case */

#endif /* GT_TEST1_H */
