/* Test header with complex GTY annotations and nested delimiters */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure */
GTY((tag("complex_struct"))) struct test_struct {
    /* Nested parentheses and brackets */
    int (* volatile arr[10])[5];
    
    /* Function pointer with complex parameter */
    void (*callback)(int (*(*nested)[5])(void));
    
    /* Multi-dimensional array with pointer */
    char *(*(*string_matrix[3])[2])[4];
};

/* Nested union/struct with bit-fields and arrays */
GTY((var, desc("%1"))) union inner_container {
    struct {
        /* Bit-field with array */
        unsigned short flags:3;
        int matrix[2][2];
        
        /* Pointer to function returning pointer to array */
        int (*(*func_ptr)(void))[10];
    } nested_struct;
    
    long long_value;
    
    /* Array of function pointers */
    double (*calc_array[5])(int, double[][3]);
};

/* Type definition with balanced delimiters in GTY arguments */
GTY((user, 
     param1 = "int (*)(int[10], char *(*(*)[5])[2])",
     param2 = "struct { int x; double y[3][3]; }"
    )) 
typedef int complex_matrix[4][4];

#endif /* GT_TEST1_H */
