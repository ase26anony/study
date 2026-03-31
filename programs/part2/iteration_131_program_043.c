/* Test header with complex nested delimiters for gengtype parser */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested delimiters */
GTY((tag("complex_struct"))) struct test_struct {
    /* Multiple levels of pointer/array nesting */
    int (* volatile arr[10])[5];
    void (* volatile (*func_ptr_array[3])(int (*(*)[5])(void)))(char);
};

/* Nested union/struct with bit-fields and multi-dimensional arrays */
GTY((desc("%1"), length("%h.count"))) union container {
    struct {
        short flags:3;
        int matrix[2][2];
        long double (*callback)(int, float[][3]);
    } inner;
    struct {
        unsigned count;
        /* Mixed delimiters in array of function pointers */
        int (*(*signal_handlers[5]))(void*);
    } meta;
    long raw;
};

/* Function pointer type with deeply nested parameter lists */
GTY((chain_next = "%h.next", chain_prev = "%h.prev")) 
struct node {
    /* Complex function pointer with array parameters */
    int (*processor)(int (*(*input)[5])(void), char output[][10][20]);
    struct node * GTY((skip)) next;
    struct node *prev;
} GTY((user));

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param1 = "int (*)(int[10])", 
     param2 = "struct { int x; float y[2][2]; }")) 
typedef int matrix_type[4][4];

#endif /* GT_TEST1_H */
