/* Test header with complex nested delimiters for gengtype parser */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested parentheses */
GTY((tag("complex_struct"))) struct test_struct {
    /* Multiple levels of pointer/array nesting */
    int (* volatile arr[10])[5];
    void (**func_ptrs[3])(int, char);
    
    /* Function pointer with complex parameter */
    int (*callback)(int (*(*array_ptr)[5])(void));
    
    /* Nested array declarations */
    char (*(*nested)[2][3])[4];
};

/* Union with nested struct containing bit-fields and arrays */
GTY((var, desc("%1"))) union inner_union {
    struct {
        short f:3;
        int g[2][2];
        long double matrix[3][3];
    } nested_struct;
    
    long h;
    
    /* Array of function pointers */
    void (*handlers[4])(int (*)(char **));
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param1 = "int (*)(int[10])", 
     param2 = "struct { int x; char y[5]; }")) 
typedef int complex_matrix[4][4];

/* Chain structure with nested delimiters */
GTY((chain_next = "%h.next", chain_prev = "%h.prev",
     desc("NODE: %1"))) 
struct chain_node {
    /* Function pointer array with parameter list containing arrays */
    void (*callbacks[2])(int [][5], char (*)[3]);
    
    /* Pointer to array of function pointers */
    int (*(*func_array)[5])(void);
    
    struct chain_node *next;
    struct chain_node *prev;
};

#endif /* GT_TEST1_H */
