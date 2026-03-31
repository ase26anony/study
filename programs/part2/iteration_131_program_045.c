/* Test header 1: Complex nested delimiters in GTY annotations */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested parentheses */
GTY((tag)) struct test_struct_1 {
    /* Multiple levels of pointer/array nesting */
    int (* volatile arr_ptr[10])[5];
    void (*func_array[3])(int (*callback)(int, char **));
};

/* Nested union/struct with bit-fields and multi-dimensional arrays */
GTY((var)) union inner_union {
    struct {
        short bitfield:3;
        int matrix[2][2];
        long (*func_ptr)(int (*)[5]);
    } nested;
    long simple;
    char (*string_array)[10];
};

/* Function pointer type with deeply nested parameter lists */
typedef GTY((chain_next = "%h.next", chain_prev = "%h.prev"))
struct complex_node {
    /* Function returning pointer to array of function pointers */
    int (*(*get_callbacks)(void))[5];
    struct complex_node *next;
    struct complex_node *prev;
    /* Array of pointers to functions taking array parameters */
    void (*handlers[4])(int params[3][2]);
} complex_node_t;

/* Edge case: unusual character that might trigger default case */
GTY((user)) typedef int matrix_type[4][4];

/* Multiple GTY markers with complex arguments */
GTY((desc("%1"), param_is = "struct test_struct_1 *"))
extern int (*(*global_callback))(void);

#endif /* GT_TEST1_H */
