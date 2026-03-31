#ifndef GT_TEST1_H
#define GT_TEST1_H

/* Test nested delimiters in GTY macro arguments */
GTY((chain_next = "%h.next", chain_prev = "%h.prev", 
     user_data = "int (*callback)(int (*(*array)[5])(void))"))
struct node {
    /* Complex pointer-to-array in structure */
    GTY((tag)) struct inner {
        int (* volatile arr[10])[5];
        void (*fn_array[2])(int [][5]);
    } data;
    
    /* Nested union with bit-fields and arrays */
    GTY((var)) union {
        struct {
            short f:3;
            int g[2][2];
            long matrix[4][4];
        };
        long h;
        double (*complex_ptr)[3][2];
    } u;
    
    struct node *next;
    struct node *prev;
};

/* Function pointer types with deeply nested parameter lists */
typedef GTY((desc("%1"))) 
int (*(*complex_func_ptr)(int, float))(char (*)[10], void (*)(int));

/* Type definition with balanced brackets in GTY arguments */
GTY((user, param1 = "int (*)(int[10][2])", 
     param2 = "struct { int x; int y[3]; }"))
typedef int matrix_type[4][4];

/* Edge case: unusual character to potentially trigger default case */
GTY((var)) int weird_type;  /* The @ symbol might trigger default case */

#endif /* GT_TEST1_H */
