#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Complex pointer-to-array in structure with nested delimiters */
GTY((tag("test_struct"))) struct test_struct {
    /* Multiple levels of pointer/array nesting */
    int (* volatile arr[10])[5];
    void (* volatile (* volatile fn_ptr_array[3]))(int, char);
    
    /* Function pointer with complex parameter list */
    int (*callback)(int (*(*)[5])(void), char (*)[3][2]);
    
    /* Nested structure with bit-fields and arrays */
    struct {
        unsigned short flags:4;
        int matrix[2][3];
        union {
            long long data;
            char bytes[8];
        } GTY((skip)) u;
    } inner;
};

/* Union with deeply nested type specifications */
GTY((var, desc("%1"))) union complex_union {
    struct {
        short f:3;
        int g[2][2];
        void (*operations[2])(int (*)[3], char (*)[5]);
    };
    long h;
    double (*compute)(int, float (*)[4][4]);
};

/* Type definition with balanced brackets in GTY arguments */
GTY((user("matrix_type"), param("dimensions = [4][4]"))) 
typedef int matrix_type[4][4];

/* Chain structure with function pointers containing nested arrays */
GTY((chain_next = "%h.next", chain_prev = "%h.prev")) 
struct node {
    int id;
    /* Function pointer returning pointer to array */
    int (*(*get_array)(void))[10];
    /* Pointer to function taking array of function pointers */
    void (*execute)(int (*callbacks[5])(int, char));
    struct node *next;
    struct node *prev;
};

#endif /* GT_TEST1_H */
