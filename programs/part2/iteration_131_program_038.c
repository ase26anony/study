/* Test header with complex GTY annotations to trigger consume_balanced calls */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure - nested brackets */
GTY((tag)) struct test_struct {
    /* Multiple levels of pointer/array nesting */
    int (* volatile arr_ptr[10])[5];
    int (*(*deep_nested)[3][2])(void);
};

/* Nested union/struct with bit-fields and arrays */
GTY((var)) union inner_union {
    struct {
        short field_bit:3;
        int matrix[2][2];
        void (*callback_array[2])(int param[][5]);
    } nested;
    long simple;
    /* Array of function pointers with complex return types */
    int (*(*func_ptr_matrix[3])[4])(char);
};

/* Function pointer types with deeply nested parameter lists */
typedef GTY((chain_next = "%h.next", chain_prev = "%h.prev")) 
struct node {
    /* Extremely complex function pointer type */
    int (*complex_callback)(
        int (*(*nested_array)[5])(
            void (*)(char, short), 
            int[3][3]
        )
    );
    struct node *next;
    struct node *prev;
} node_t;

/* Type with multiple balanced delimiters in GTY arguments */
GTY((user, 
     param1 = "int (*)(int[10], char (*)[5])",
     param2 = "struct { int x; void (*f)(int[][2]); }"
)) 
typedef int complex_matrix[4][4];

#endif /* GT_TEST1_H */
