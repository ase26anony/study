#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structures */
GTY((tag)) struct test_struct {
    int (* volatile arr[10])[5];
    void (*complex_func_ptr)(int (*)[3][4]);
};

/* Nested unions/structs with bit-fields and arrays */
GTY((var)) union inner_union {
    struct {
        short f:3;
        int g[2][2];
        long double matrix[3][3];
    } nested;
    long h;
    struct {
        int (*callback_array[2])(int, char);
    } funcs;
};

/* Function pointer types with deeply nested parameter lists */
typedef GTY((chain_next = "%h.next")) struct node {
    int (*callback)(int (*(*array_ptr)[5])(void));
    struct node *next;
    void (*signal_handler)(int sig, void (*cleanup)(void *));
} node_t;

/* Type with multiple levels of nested parentheses */
GTY((user)) typedef int (*(*complex_type_def))(int (*params)[10], 
                                               char (*(*names)[5])(void));

/* Edge case: unusual character that might trigger default case */
GTY((var)) int weird@type;  /* This '@' should trigger default: case */

#endif /* GT_TEST1_H */
