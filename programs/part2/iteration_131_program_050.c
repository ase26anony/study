/* Test header with complex GTY annotations to trigger consume_balanced logic */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested parentheses */
GTY((tag("complex_array"))) struct test_struct {
    /* Multi-dimensional array of function pointers */
    int (*(*volatile arr[10]))[5];
    
    /* Nested structure with bit-fields */
    struct {
        unsigned int flags:8;
        int (*callback)(int, char **);
    } GTY((skip)) inner;
    
    /* Array of pointers to arrays */
    int *(*(*matrix_ptr)[3])[4];
};

/* Function pointer type with deeply nested parameter lists */
typedef GTY((chain_next = "%h.next")) struct node {
    /* Complex function pointer with array parameter */
    int (*complex_callback)(int (*(*callback_array)[5])(void), 
                           struct node ***node_list);
    
    /* Pointer to array of function pointers returning pointers */
    void *(*(*func_table)[2][3])(int, ...);
    
    struct node *next;
} node_t GTY((user));

/* Union with nested structures and arrays */
GTY((var, desc("%1"))) union container {
    /* Anonymous struct with 2D array */
    struct {
        short field1:3;
        int field2[2][2];
        long (*func_ptr)(double matrix[3][3]);
    };
    
    /* Another struct with pointer to volatile array */
    struct {
        volatile int *volatile_ptr_array[5];
        char (*(*string_matrix)[10])[20];
    } GTY((tag)) named;
    
    long simple;
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param1 = "int (*)(int[10][5])", 
     param2 = "struct { int x; double y[2]; }")) 
typedef int complex_matrix[4][4];

#endif /* GT_TEST1_H */
