#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Test nested parentheses in GTY arguments */
GTY((chain_next = "%h.next", chain_prev = "%h.prev", 
     user_data = "int (*callback)(int (*(*array)[5])(void))"))
struct complex_node {
    /* Deeply nested pointer-to-array in structure */
    GTY((tag)) struct inner {
        int (* volatile arr[10])[5];
        void (* volatile fn_ptr)(int (*)(int[3][3]));
    } data;
    
    /* Function pointer with complex parameter list */
    int (*callback)(int (*(*param)[5])(void), 
                    struct inner (*process)(int[][7]));
    
    struct complex_node *next;
    struct complex_node *prev;
};

/* Mixed delimiters in type definition */
GTY((var)) union mixed_delimiters {
    struct {
        short field1:3;
        int matrix[2][(3 + 2)];  /* Parentheses in array dimension */
        long (*func_array[2])(int [][5]);
    };
    long value;
    /* Nested union with bit-fields */
    union {
        unsigned bits:4;
        char str[10];
    } nested;
};

/* Test the default case with unusual characters */
GTY((var)) int weird@type;  /* This should trigger default: case */

#endif /* GT_TEST1_H */
