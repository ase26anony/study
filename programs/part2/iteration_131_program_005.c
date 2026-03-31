/* Test header with complex GTY annotations and nested delimiters */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure */
GTY((tag("test_struct"))) struct test_struct {
    /* Nested array of function pointers with complex return types */
    int (*(* volatile arr[10]))[5];
    
    /* Function pointer with nested parameter lists */
    void (*callback)(int (*(*)[5])(void), char *(*)(int[][3]));
    
    /* Multi-dimensional array with nested brackets */
    unsigned char matrix[4][4][2];
};

/* Nested union/struct with bit-fields and arrays */
GTY((var, desc("%1"))) union inner_union {
    struct {
        /* Bit-field followed by nested array */
        short field1:3;
        int field2[2][2];
        
        /* Pointer to array of function pointers */
        void (*(*func_ptr_array)[3])(int, ...);
    } nested_struct;
    
    long long_value;
    
    /* Array of pointers to functions returning pointers to arrays */
    char (*(*string_array[5])[10])(void);
};

/* Type definition with deeply nested brackets */
GTY((user)) typedef int (*(*complex_matrix_ptr)[4])[4];

/* Chain structure with nested delimiters in attributes */
GTY((chain_next = "%h.next", chain_prev = "%h.prev",
     desc("Node with complex function pointer type")))
struct node {
    /* Function pointer with array parameter and nested parentheses */
    int (*processor)(int (*callback)(int[][5], char *), 
                     void *context,
                     struct { int x; double y; } data);
    
    struct node *next;
    struct node *prev;
    
    /* Anonymous union with bitfields */
    union {
        struct {
            unsigned flag1:1;
            unsigned flag2:2;
            unsigned :5;  /* Unnamed bitfield */
        } bits;
        unsigned char byte;
    } flags;
};

#endif /* GT_TEST1_H */
