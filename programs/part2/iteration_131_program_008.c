/* Test header with complex GTY annotations and nested delimiters */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with deeply nested delimiters */
GTY((tag("complex_struct"))) struct test_struct {
    /* Multi-level pointer to array with volatile qualifier */
    int (* volatile arr[10])[5];
    
    /* Function pointer array with nested parameter lists */
    void (*callbacks[3])(int (*)(char *), double);
    
    /* Nested anonymous struct with bit-fields */
    struct {
        unsigned int flags:8;
        signed int count:16;
        long (*matrix_ptr)[4][4];
    } GTY((skip)) inner;
};

/* Union with nested struct containing 2D arrays */
GTY((var, desc("%1"))) union data_union {
    struct {
        short bits:3;
        int matrix[2][2];
        char (*string_array[5])[10];
    } s;
    long value;
    void *(*func_ptr)(int [][5], char (*)[10]);
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param1 = "int (*callback)(int (*(*)[5])(void))")) 
typedef struct node node_t;

/* Multiple GTY markers with complex chain_next expression */
GTY((chain_next = "%h.next", chain_prev = "%h.prev", 
     desc("%1: custom desc with [brackets] and {braces}")))
struct node {
    int value;
    /* Function pointer type with nested array parameters */
    int (*processor)(int data[][3][3], void (*handler)(char));
    node_t *next;
    node_t *prev;
};

/* Edge case: unusual character to potentially trigger default case */
GTY((var)) extern int weird@type;  /* @ symbol might trigger default case */

#endif /* GT_TEST1_H */
