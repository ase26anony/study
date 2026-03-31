/* Test header with complex nested delimiters for gengtype parser */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested parentheses */
GTY((tag("complex_array"))) struct test_struct {
    /* Multiple levels of pointer/array nesting */
    int (* volatile arr[10])[5];
    void (*func_ptr)(int (*callback)(int, char **));
};

/* Nested union/struct with bit-fields and multi-dimensional arrays */
GTY((var, desc("%1"))) union inner_container {
    struct {
        short field1:3;
        int field2[2][2];
        long (*matrix_ptr)[4][4];
    } nested;
    long simple;
    /* Function pointer array with parameter list */
    int (*(*func_array[3]))(void (*)(int));
};

/* Type with deeply nested balanced delimiters */
GTY((user)) typedef int (*(*complex_type))(int (*(*)[5])(void), char [10][20]);

#endif /* GT_TEST1_H */
