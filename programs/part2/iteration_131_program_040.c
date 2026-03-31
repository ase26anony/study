/* Test header with complex nested delimiters for gengtype parser coverage */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested parentheses */
GTY((tag("complex_struct"))) struct test_struct {
    /* Multi-level pointer to array with volatile qualifier */
    int (* volatile arr_ptr[10])[5];
    
    /* Function pointer with array parameter */
    void (*callback)(int param[][5]);
    
    /* Nested structure with bit-fields */
    struct {
        unsigned int flag:1;
        unsigned int count:7;
        int matrix[2][3];
    } GTY((skip)) inner;
};

/* Union with deeply nested type */
GTY((var, desc("%1"))) union complex_union {
    /* Anonymous struct with array of function pointers */
    struct {
        int (*(*func_array[2]))(void);
        short bits:4;
        int nested_arr[2][2];
    };
    
    /* Pointer to array of pointers */
    long (*(*long_ptr_arr)[5]);
    
    /* Multi-dimensional array */
    double deep_array[3][4][2];
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user("matrix_type"), param("int[4][4]"))) 
typedef int matrix_t[4][4];

/* Function pointer type with complex parameter list */
GTY((chain_next = "%h.next")) 
typedef struct node {
    /* Callback taking pointer to array of function pointers */
    int (*handler)(int (*(*callback_array)[5])(void));
    
    /* Nested pointer type with parentheses */
    void (*(*nested_fn_ptr))(int, char *);
    
    struct node *next;
} node_t;

/* Edge case: unusual character to potentially trigger default case */
GTY((var)) extern int weird@type;  /* '@' might trigger default: advance() */

#endif /* GT_TEST1_H */
