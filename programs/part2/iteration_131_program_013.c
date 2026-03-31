/* Test header with complex GTY annotations and nested delimiters */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested parentheses */
GTY((tag)) struct test_struct {
    /* Deeply nested function pointer with array parameters */
    int (*(* volatile complex_func_ptr[10]))(int (*)[5], char (*(*)[3])[2]);
    
    /* Multi-dimensional array of function pointers */
    void (*(*array_of_funcs[2][3]))(int, ...);
    
    /* Nested structure with bit-fields */
    struct {
        unsigned int flag:1;
        int (*(*nested_array)[4])[2]; /* Pointer to array of pointers to arrays */
    } inner;
};

/* Union with deeply nested type */
GTY((var)) union complex_union {
    /* Anonymous struct with complex type */
    struct {
        short bits:3;
        int matrix[2][2];
        /* Function pointer returning pointer to array */
        int (*(*get_matrix)(void))[2][2];
    };
    
    long simple;
    
    /* Pointer to function with complex parameter */
    void (*callback)(int (*(*param)[5])(void), struct test_struct*);
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param_type = "int (*)(int[10][10])")) 
typedef int (*(*complex_typedef))(int matrix[][10]);

/* Chain structure with nested delimiters */
GTY((chain_next = "%h.next", chain_prev = "%h.prev"))
struct chain_node {
    /* Array of function pointers with nested parameter lists */
    int (*(*handlers[3]))(int, char (*)[2], void (*)(double));
    
    /* Pointer to array of pointers */
    int (**ptr_array)[4];
    
    struct chain_node *next;
    struct chain_node *prev;
};

#endif /* GT_TEST1_H */
