#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested delimiters */
GTY((tag("test_struct"))) struct test_struct {
    /* Multiple levels of nested parentheses and brackets */
    int (* volatile arr[10])[5];
    void (*complex_func)(int (*callback)(int (*(*)[5])(void)));
    
    /* Nested union with struct containing bit-fields and arrays */
    GTY((skip)) union inner_union {
        struct {
            short f:3;
            int g[2][2];
            long (*h[3])(int[][5]);
        } nested;
        long long value;
    } data;
    
    /* Function pointer with complex parameter list */
    char (*(*string_table)[10])(int, char (*)[20]);
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param1 = "int (*)(int[10][5])", 
     param2 = "struct { int x; double y[3]; }")) 
typedef int matrix_type[4][4];

/* Chain structure with nested delimiters in chain_next expression */
GTY((chain_next = "%h.next", chain_prev = "%h.prev")) 
struct chain_node {
    int value;
    /* Complex array of function pointers */
    void (*(*func_ptrs[2][3]))(int (*)[5], char **);
    struct chain_node *next;
    struct chain_node *prev;
};

#endif /* GT_TEST1_H */
