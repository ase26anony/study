#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structures */
GTY((tag("test_struct"))) struct test_struct {
    /* Nested parentheses and brackets */
    int (* volatile arr[10])[5];
    /* Function pointer with complex parameter */
    void (*callback)(int (*(*array_ptr)[5])(void));
};

/* Nested union/struct with bit-fields and arrays */
GTY((var)) union inner_union {
    struct {
        short f:3;
        /* Multi-dimensional array */
        int g[2][2];
        /* Pointer to array of function pointers */
        int (*(*func_array)[3])(int, char);
    };
    long h;
    /* Array of pointers to arrays */
    double *(*matrix_ptr)[4][4];
};

/* Type with deeply nested delimiters in GTY arguments */
GTY((chain_next = "%h.next", 
     chain_prev = "%h.prev",
     desc("%1: complex nested type"))) 
struct node {
    /* Function pointer returning pointer to array */
    int (*(*get_matrix)(void))[10][10];
    struct node *next;
    struct node *prev;
};

/* Edge case: unusual character to potentially trigger default case */
GTY((var)) int weird@type;  /* '@' might trigger default: case */

#endif /* GT_TEST1_H */
