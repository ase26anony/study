#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested delimiters */
GTY((tag)) struct test_struct {
    /* Multiple levels of parentheses and brackets */
    int (* volatile arr[10])[5];
    void (*complex_func_ptr)(int (*)(int[3][4]), char *);
};

/* Nested union/struct with bit-fields and multi-dimensional arrays */
GTY((var)) union inner_union {
    struct {
        short f:3;
        int g[2][2];
        long double matrix[3][3];
    };
    long h;
    void (*callback_array[2])(int, char **);
};

/* Function pointer type with deeply nested parameter lists */
typedef GTY((chain_next = "%h.next", chain_prev = "%h.prev")) 
struct node {
    /* Extremely complex function pointer type */
    int (*callback)(int (*(*array_ptr)[5])(void), 
                    struct { int x; double y; } (*)(char[10]));
    
    /* Pointer to array of function pointers */
    void (*(*func_ptr_array[3]))(int (*)[2][2]);
    
    struct node *next;
    struct node *prev;
} node_t;

/* Type with balanced brackets in GTY arguments */
GTY((user, param1 = "int (*)(int[10][20])", 
     desc("%1 uses nested arrays"))) 
typedef int complex_matrix[4][4][4];

/* Edge case: unusual character to potentially trigger default case */
GTY((var)) int weird@type;  /* '@' character might trigger default */

#endif /* GT_TEST1_H */
