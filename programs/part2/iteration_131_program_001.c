#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested delimiters */
GTY((tag("complex_struct"))) struct test_struct {
    /* Multiple levels of nested parentheses and brackets */
    int (* volatile arr[10])[5];
    void (*callback_array[3])(int (*)(char (*)[7]), double);
    struct {
        /* Nested braces and brackets */
        union {
            short bits:4;
            int matrix[2][(3+2)];
        } inner;
        /* Function pointer with complex parameter */
        long (*processor)(int (*(*signal)[5])(void), float);
    } nested;
};

/* Nested union/struct with bit-fields and arrays */
GTY((var, desc("%1"))) union container {
    struct {
        short f:3;
        int g[2][2];
        /* More nested delimiters */
        char (*strings[4])[(sizeof(int)*2)];
    };
    long h;
    /* Array of function pointers */
    void (*(*func_table)[8])(int, ...);
};

/* Chain structure with complex next pointer type */
typedef GTY((chain_next = "%h.next", chain_prev = "%h.prev")) 
struct node {
    /* Extremely complex function pointer type */
    int (*callback)(int (*(*array_ptr)[5])(void), 
                    char (*(*str_matrix)[3][3])[10]);
    struct node *next;
    struct node *prev;
    /* Multi-dimensional array with computed size */
    double data[sizeof(struct node*) > 4 ? 8 : 4][(2+3)*2];
} node_t;

/* Type with deeply nested delimiters in GTY arguments */
GTY((user, 
     param1 = "int (*)(int[10], char (*(*)[5])[3])",
     param2 = "struct { int a; double b[(2+3)]; }"))
typedef int hyper_matrix[4][4][2];

#endif /* GT_TEST1_H */
