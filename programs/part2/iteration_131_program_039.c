/* Test header with complex nested delimiters for gengtype parser */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested parentheses */
GTY((tag("complex_struct"))) struct test_struct {
    /* Multiple levels of pointer/array nesting */
    int (* volatile arr[10])[5];
    void (**func_ptrs[3])(int, char);
    
    /* Function pointer with array parameter */
    int (*callback)(int (*array_param)[5][10]);
};

/* Nested union/struct with bit-fields and multi-dimensional arrays */
GTY((var, desc("%1"))) union inner_container {
    struct {
        short flags:3;
        int matrix[2][2];
        long (*ptr_matrix[3])[4];
    } nested;
    
    long simple_value;
    
    /* Another nested structure */
    struct {
        char *name;
        int (*compare)(const void *, const void *);
    } comparator;
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param_is = "int (*)(int[10])")) 
typedef int (*complex_func_ptr)(int array[10]);

#endif /* GT_TEST1_H */
