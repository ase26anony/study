/* Test header with complex GTY annotations and nested delimiters */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with deeply nested parentheses */
GTY((tag("complex_array"))) struct test_struct {
    /* Multi-level pointer to array with volatile qualifier */
    int (* volatile arr_ptr[10])[5];
    
    /* Function pointer array with nested parameter lists */
    void (*callbacks[3])(int (*)(char (*)[10]), double);
    
    /* Nested anonymous struct with bit-fields */
    struct {
        unsigned short flags:4;
        int matrix[2][2];
        union {
            long long data;
            struct { char a; int b; } inner;
        } u GTY((skip));
    } nested;
};

/* Union with nested struct containing arrays and bit-fields */
GTY((var, desc("%1"))) union container {
    struct {
        short field1:3;
        int field2[2][2];
        /* Nested array of function pointers */
        int (*(*func_array)[5])(void);
    } s;
    long long value;
    /* Multi-dimensional array */
    double matrix[3][3][3];
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user("matrix_type"), param("int[4][4]"))) 
typedef int complex_matrix[4][4];

/* Chain structure with function pointer containing complex type */
GTY((chain_next = "%h.next", chain_prev = "%h.prev")) 
struct node {
    /* Function pointer returning pointer to array */
    int (*(*get_matrix)(void))[5];
    
    /* Callback with nested array parameter */
    void (*processor)(int data[][5][2]);
    
    struct node *next;
    struct node *prev;
} GTY((tag("node_type")));

/* External declaration with complex pointer type */
GTY((var)) extern int (*(*global_callback))(int (*)[10]);

#endif /* GT_TEST1_H */
