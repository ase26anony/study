/* Test header with complex nested delimiters for gengtype parser */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested parentheses */
GTY((tag("complex_struct"))) struct test_struct {
    /* Multiple levels of pointer/array nesting */
    int (* volatile arr_ptr[10])[5];
    void (**func_table)(int, char*);
    
    /* Function pointer with complex parameter */
    int (*comparator)(int (*(*callback_array)[5])(void), int);
    
    /* Nested anonymous struct with bit-fields */
    struct {
        unsigned short flags:3;
        signed int :4;  /* Unnamed bit-field */
        unsigned long value:16;
    } GTY((skip)) bits;
};

/* Union with deeply nested array types */
GTY((var, desc("%1"))) union data_container {
    /* 3D array with function pointer elements */
    int (*(*matrix3d[2][3][4]))(double);
    
    struct {
        /* Mixed delimiters: array of pointers to arrays */
        float *(*(*nested_arr)[7])[3];
        
        /* Complex function pointer type */
        char *(*(*string_processor)(int, ...))(const char*);
    } GTY((user)) complex_member;
    
    /* Simple member to trigger different paths */
    long simple;
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user("matrix_type"), 
     param1 = "int (*)(int[10][20])",
     param2 = "void (*[5])(struct test_struct*)"))
typedef int multi_array[4][4][4];

/* Chain structure with nested delimiters in chain_next */
GTY((chain_next = "%h.next->data->arr_ptr[0]",
     chain_prev = "%h.prev"))
struct chain_node {
    struct test_struct *data;
    struct chain_node *next;
    struct chain_node *prev;
    
    /* Array of function pointers with complex return type */
    struct chain_node *(*(*node_ops[3])(int))(void);
};

#endif /* GT_TEST1_H */
