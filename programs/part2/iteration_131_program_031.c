/* Test header with complex GTY annotations and nested delimiters */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with deeply nested parentheses */
GTY((tag)) struct test_struct {
    /* Multiple levels of pointer/array nesting */
    int (* volatile arr[10])[5];
    
    /* Function pointer with complex parameter list */
    void (*callback)(int (*(*complex_param)[5])(void));
    
    /* Nested structure with bit-fields and arrays */
    struct {
        unsigned short flags:3;
        int matrix[2][2];
        long (*func_ptr_array[3])(int, char);
    } GTY((skip)) inner;
};

/* Union with nested anonymous struct containing arrays */
GTY((var)) union data_union {
    struct {
        short f:3;
        int g[2][2];
        char (*string_array[4])[10];
    };
    long h;
    double (*compute)(int matrix[3][3], void (*cb)(void));
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param1 = "int (*)(int[10])", 
     param2 = "struct { int x; char y[5]; }")) 
typedef int complex_matrix[4][4];

/* Chain structure with function pointer types */
GTY((chain_next = "%h.next", chain_prev = "%h.prev",
     desc("%1"), length("%1.count")))
struct node {
    /* Function pointer returning pointer to array */
    int (*(*get_matrix)(void))[5][5];
    
    /* Nested function pointer with parameter list */
    void (*processor)(int (*filter)(int arr[][10]), 
                      char (*names)[20][30]);
    
    struct node *next;
    struct node *prev;
    int count;
};

/* Edge case: unusual character that might trigger default case */
GTY((var)) int normal_var;

/* This might trigger the default case with '@' symbol */
#ifdef TEST_EDGE_CASE
GTY((var)) int weird@type;  /* Unusual character in type name */
#endif

#endif /* GT_TEST1_H */
