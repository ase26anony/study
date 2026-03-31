/* Test header with complex nested delimiters for gengtype parser */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with deeply nested brackets */
GTY((tag)) struct test_struct {
    /* Multi-dimensional array of function pointers */
    int (*(*volatile arr[10])[5])(void);
    
    /* Nested array with multiple dimensions */
    char matrix[4][4][2];
    
    /* Function pointer with complex parameter */
    void (*callback)(int (*)(int[][5], char **));
};

/* Union with nested struct containing bit-fields and arrays */
GTY((var)) union inner_union {
    struct {
        short f:3;
        int g[2][2];
        long double h[3];
    } nested;
    long long value;
    
    /* Array of pointers to arrays */
    int *(*(*complex_ptr_arr)[3])[2];
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param = "int (*)(int[10][20])")) 
typedef struct {
    int (*processor)(int matrix[][10]);
    float data[5][5];
} matrix_processor_t;

#endif /* GT_TEST1_H */
