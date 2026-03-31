/* Test header with complex nested delimiters for gengtype parser */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested parentheses */
GTY((tag)) struct test_struct {
    /* Multiple levels of pointer/array nesting */
    int (* volatile arr[10])[5];
    void (**func_ptr_array)(int, char);
    
    /* Function pointer with complex parameter */
    int (*callback)(int (*(*complex_param)[5])(void));
};

/* Nested union/struct with bit-fields and arrays */
GTY((var)) union inner_union {
    struct {
        short field1:3;
        short field2:5;
        int matrix[2][2];
        char (*string_array[4])[10];
    } nested;
    long long_value;
    
    /* Another nested structure */
    struct {
        int (*func)(int[][3], char (*)[5]);
        union {
            float f;
            double d;
        } value;
    } another;
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param1 = "int (*)(int[10])", 
     param2 = "struct { int x; int y; }")) 
typedef int complex_matrix[4][4];

/* Chain structure with function pointers */
GTY((chain_next = "%h.next", chain_prev = "%h.prev",
     desc("%1"), length("%1.count")))
struct chain_node {
    /* Function pointer array with nested parameter types */
    void (*handlers[3])(int (*)(char[][10]), float);
    
    /* Pointer to array of function pointers */
    int (*(*callback_array)[5])(void);
    
    struct chain_node *next;
    struct chain_node *prev;
    int count;
};

/* Edge case: unusual character to trigger default case */
GTY((var)) int weird@type;  /* '@' should trigger default: advance() */

/* Multi-dimensional function pointer */
GTY((tag)) struct func_container {
    /* Triple pointer to function returning pointer to array */
    int (*(***(*triple_func))(void))[10];
    
    /* Nested structure with array of pointers */
    struct {
        char *(*names[5])[20];
        int values[3][3][3];
    } data;
};

#endif /* GT_TEST1_H */
