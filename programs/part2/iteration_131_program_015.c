/* Test header with complex nested delimiters for gengtype parser coverage */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure */
GTY((tag("test_struct"))) struct test_struct {
    /* Nested array of function pointers with complex parameters */
    int (*(*volatile arr[10]))[5];
    
    /* Multi-dimensional array with nested parentheses */
    void (*callback_array[3])(int (*)(char (*)[10]), double);
    
    /* Deeply nested combination */
    struct {
        union {
            int (*matrix_func)(int [][5][5]);
            long (*vector_func[2])(void);
        } u;
        short bits:4;
    } nested;
};

/* Function pointer type with deeply nested parameter lists */
typedef GTY((chain_next = "%h.next")) struct node {
    /* Extremely complex function pointer type */
    int (*(*(*complex_callback))(int (*(*)[5])(void), 
                                 struct { int x; double y; }))[10];
    
    /* Array of pointers to functions returning pointers to arrays */
    float (*(*func_table[2][3])(int))[7];
    
    struct node *next;
} node_t;

/* Union with bit-fields and nested arrays */
GTY((var)) union inner_union {
    struct {
        /* Bit-field followed by multi-dimensional array */
        short field1:3;
        unsigned int field2:5;
        int matrix[2][2][2];
        
        /* Pointer to array of function pointers */
        char (*(*string_proc)[10])(const char *);
    } s;
    
    long long_data;
    
    /* Anonymous struct with nested delimiters */
    struct {
        int (*compare)(const void *, const void *);
        void (*sort[2])(int [], size_t);
    };
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user, 
     param1 = "int (*)(int[10][10], char (*)[5])",
     param2 = "struct { int x; double y; }")) 
typedef int complex_matrix[4][4][4];

/* Edge case: unusual character to potentially trigger default case */
GTY((var)) extern int weird@type;  /* '@' might trigger default case */

#endif /* GT_TEST1_H */
