/* Test header with complex nested delimiters for gengtype parser */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested parentheses */
GTY((tag("complex_struct"))) struct test_struct {
    /* Multiple levels of pointer/array nesting */
    int (* volatile arr_ptr[10])[5];
    void (*(*func_ptr_array[3]))(int, char);
    
    /* Function pointer with array parameter */
    int (*callback)(int matrix[4][4], void (*handler)(void));
};

/* Nested union/struct with bit-fields and multi-dimensional arrays */
GTY((desc("%0"))) union container {
    struct {
        /* Bit-field with array */
        unsigned short flags:4;
        int matrix[2][3][4];
        
        /* Pointer to array of function pointers */
        int (*(*func_table)[5])(void);
    } inner;
    
    /* Union alternative with different nesting */
    long data;
    
    /* Anonymous struct with more nesting */
    struct {
        char (*string_array[8])[20];
        void (*actions[2])(int (*)(int[][5]));
    };
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param_is = "int (*)(int[10])")) 
typedef int (*array_func_ptr)(int arr[10]);

/* Chain structure with complex next pointer type */
GTY((chain_next = "%h.next", chain_prev = "%h.prev"))
struct chain_node {
    /* Complex function pointer type with nested parameter list */
    void (*(*signal_handler[2]))(int (*(*callback_array)[5])(void));
    
    /* Multi-dimensional pointer array */
    int (*(*volatile triple_ptr)[3][2])[4];
    
    struct chain_node *next;
    struct chain_node *prev;
};

#endif /* GT_TEST1_H */
