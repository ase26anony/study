/* Test header with complex nested delimiters for gengtype parser */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested parentheses */
GTY((tag("complex_struct"))) struct test_struct {
    /* Multiple levels of pointer/array nesting */
    int (* volatile arr[10])[5];
    void (*func_ptr_array[3])(int (*)(char **));
};

/* Nested union/struct with bit-fields and multi-dimensional arrays */
GTY((var, desc("%1"))) union inner_container {
    struct {
        short flags:3;
        int matrix[2][2];
        long (*callback)(int (*(*)[5])(void));
    } nested;
    long long value;
    char * GTY((skip)) skip_field;
};

/* Function pointer type with deeply nested parameter lists */
typedef GTY((chain_next = "%h.next", chain_prev = "%h.prev")) 
struct linked_node {
    /* Complex function pointer with array of function pointers as parameter */
    int (*processor)(int (*(*handlers[5]))(void), char *args[]);
    struct linked_node *next;
    struct linked_node *prev;
    /* Array of pointers to arrays */
    int *(*(*complex_array)[10])[5];
} node_t;

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param1 = "int (*)(int[10])", 
     param2 = "struct { int x; int y[2][2]; }")) 
typedef int multi_matrix[4][4];

#endif /* GT_TEST1_H */
