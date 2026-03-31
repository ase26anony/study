/* Test header with complex nested delimiters in GTY annotations */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested parentheses */
GTY((tag)) struct test_struct {
    /* Multiple levels of pointer/array nesting */
    int (* volatile arr[10])[5];
    void (**func_ptrs)(int, char);
};

/* Nested union/struct with bit-fields and multi-dimensional arrays */
GTY((var)) union inner_union {
    struct {
        short field1:3;
        short field2:5;
        int matrix[2][2];
        long (*callback)(int (*)(void), char[10]);
    };
    long long_value;
    void *ptr_array[5];
};

/* Function pointer type with deeply nested parameter lists */
typedef GTY((chain_next = "%h.next")) struct node {
    /* Complex function pointer returning function pointer to array */
    int (*(*complex_callback))(int (*(*)[5])(void), char (*)[10]);
    
    /* Nested structure with array of function pointers */
    struct {
        void (*handlers[3])(int, ...);
        int data[4][4];
    } nested;
    
    struct node *next;
} node_t;

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param1 = "int (*)(int[10], char[][5])")) 
typedef int complex_matrix[4][4];

/* Edge case: unusual character to trigger default case */
GTY((var)) int weird@type;  /* '@' should trigger default: advance() */

#endif /* GT_TEST1_H */
