/* Test header with complex GTY annotations to trigger consume_balanced calls */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested parentheses */
GTY((tag("complex_array"))) struct test_struct {
    /* Multi-level pointer to array with volatile qualifier */
    int (* volatile arr[10])[5];
    
    /* Function pointer with array parameter */
    void (*callback)(int matrix[3][3]);
    
    /* Nested pointer madness */
    char (*(*(*nested_ptr)[5])(void))[10];
};

/* Union with nested struct containing bit-fields and arrays */
GTY((var, desc("%1"))) union inner_union {
    struct {
        short f:3;
        int g[2][2];
        long double matrix[3][3][3];
    } nested;
    
    /* Array of function pointers */
    int (*(*func_array[5])(int, char))[10];
    
    long h;
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param1 = "int (*)(int[10])", 
     param2 = "struct { int x; double y[5]; }")) 
typedef int complex_matrix[4][4];

#endif /* GT_TEST1_H */
