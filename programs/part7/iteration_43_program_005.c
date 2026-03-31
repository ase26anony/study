/* { dg-do compile } */
/* Primary header with complex nested delimiter patterns */

#ifndef FILE1_H
#define FILE1_H

/* 1. Function pointers with nested argument lists */
typedef int (*callback_t)(int (*)(char), double);
typedef void (*(*signal_handler_t)(int sig, void (*)(int)))(int);

/* 2. Multi-dimensional arrays with complex size expressions */
typedef int matrix_t[10][(sizeof(int) > 4) ? 5 : 3];
typedef char vla_t[][(16 * sizeof(long))];

/* 3. Struct with flexible array member */
struct flexible_array {
    int len;
    double data[];
};

/* 4. Deeply nested function pointer */
typedef int (*(*(*deep_nested_fp)(void))[5])(int, int);

/* 5. Struct containing array of function pointers */
struct operations {
    int (*ops[5])(int, int);
    void (*handlers[])(void);
};

/* 6. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]
DECLARE_COMPLEX(10);
DECLARE_COMPLEX(20);

/* 7. Union with nested struct initializer pattern */
union nested_union {
    struct {
        int x;
        int y[2][3];
    } point;
    double matrix[2][(8/sizeof(int))];
};

/* 8. Typedef chain with all delimiter types */
typedef struct node {
    struct node *next;
    void (*process)(struct node *n);
    int values[][(sizeof(struct node*) + 2)];
} node_t;

/* 9. Function returning pointer to array of function pointers */
int (*(*get_operations(void))[5])(int, int);

/* 10. Const-volatile qualified function pointer */
typedef void (*(* const cv_fp)(volatile int))[10];

#endif /* FILE1_H */
