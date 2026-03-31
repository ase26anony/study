/* Test header with complex GTY annotations and nested delimiters */
#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure */
GTY((tag("TEST_STRUCT"), desc("%1"))) 
struct test_struct {
    /* Nested array of function pointers with complex return types */
    int (* volatile arr[10])[5];
    
    /* Multi-dimensional array with nested parentheses */
    void (*callback_array[2])(int (*)(int[][5]));
    
    /* Mixed delimiters in type */
    struct {
        union {
            int x;
            long y;
        } nested_union[3][2];
    } inner_struct;
};

/* Nested union/struct with bit-fields and arrays */
GTY((var, length("%h.count"))) 
union complex_union {
    struct {
        /* Bit-field with array */
        short flags:3;
        int matrix[2][2];
        
        /* Function pointer with nested parameter list */
        char* (*formatter)(int, char* (*)(void));
    } data;
    
    long raw_value;
    void* ptr_array[4];
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param("int (*callback)(int (*(*)[5])(void))"))) 
typedef int complex_matrix[4][4];

#endif /* GT_TEST1_H */
