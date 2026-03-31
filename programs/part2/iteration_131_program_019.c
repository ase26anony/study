/* Test header with complex nested delimiters for gengtype parser */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested parentheses */
GTY((tag("complex_array"))) struct test_struct {
    /* Multiple levels of pointer/array nesting */
    int (* volatile arr_ptr[10])[5];
    void (*func_array[3])(int (*callback)(int, char **));
    
    /* Nested structure with bit-fields and arrays */
    struct {
        unsigned short flags:4;
        int matrix[2][3];
        union {
            long long big_val;
            struct { char a; int b; } nested;
        } data_union;
    } inner GTY((skip));
};

/* Function pointer type with deeply nested parameter lists */
typedef GTY((chain_next = "%h.next", chain_prev = "%h.prev")) 
struct node {
    /* Complex function pointer returning function pointer to array */
    int (*(*complex_callback)(int (*(*param)[5])(void)))(float);
    
    /* Array of function pointers with array parameters */
    void (*handlers[4])(int data[][5], char *names[]);
    
    struct node *next;
    struct node *prev;
} node_t;

/* Union with anonymous struct containing arrays */
GTY((var, desc("%1"))) union container {
    struct {
        short bits:3;
        int grid[2][2];
        char *strings[];
    };
    long value;
    void *ptr_array[4];
};

#endif /* GT_TEST1_H */
