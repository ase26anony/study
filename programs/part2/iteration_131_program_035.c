/* Test header with nested delimiters in GTY annotations */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure */
GTY((tag("TEST_STRUCT"))) struct test_struct {
    /* Nested array of function pointers with complex parameters */
    int (* volatile arr[10])[5];
    
    /* Function pointer with nested parameter list */
    void (*callback)(int (*(*nested)[5])(void));
    
    /* Multi-dimensional array with nested braces */
    int matrix[3][2][4];
};

/* Nested union/struct with bit-fields and arrays */
GTY((var, desc("%1"))) union inner_union {
    struct {
        short f:3;
        int g[2][2];
        long h:7;
    } nested_struct;
    
    /* Array within union */
    long long array[3][3];
    
    /* Function pointer array */
    int (*func_ptrs[2])(int, char);
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param1 = "int (*)(int[10])", 
     param2 = "struct { int x; double y[2]; }")) 
typedef int complex_matrix[4][4];

/* Chain structure with nested delimiters */
GTY((chain_next = "%h.next", chain_prev = "%h.prev",
     desc("NODE: %1"))) 
struct node {
    /* Complex function pointer type */
    int (*processor)(int (*)(char (*)[5]), void*);
    
    /* Pointer to array of pointers */
    struct node** (*get_children)(void)[5];
    
    struct node *next;
    struct node *prev;
};

/* Edge case: unusual characters that might trigger default case */
GTY((var)) extern int weird@type;  /* '@' character in identifier */

#endif /* GT_TEST1_H */
