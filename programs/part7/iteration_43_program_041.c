/* { dg-do compile } */
/* Primary header with complex nested delimiter patterns */

#ifndef FILE1_H
#define FILE1_H

/* 1. Function pointers with nested argument lists */
typedef int (*callback_t)(int (*)(char), double);
typedef void (*(*signal_handler_t)(int sig, void (*func)(int)))(int);

/* 2. Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 0 ? (x) : 1)
extern int matrix[10][DYNAMIC_SIZE(5)];
typedef int vla_t[DYNAMIC_SIZE(sizeof(int) * 8)];

/* 3. Struct with flexible array member containing function pointers */
struct container {
    int len;
    int (*funcs[])(int, int);
};

/* 4. Deeply nested function pointer returning pointer to array */
typedef int (*(*complex_callback_t)(void))[10];

/* 5. Struct containing array of function pointers with nested prototypes */
struct operations {
    int (*ops[5])(int (*)(char), double);
    void (*handlers[3])(struct operations *);
};

/* 6. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]
DECLARE_COMPLEX(5);
DECLARE_COMPLEX(10);

/* 7. Union with nested struct and array */
union nested_data {
    struct {
        int x;
        int y[3];
    } point;
    struct {
        char *name;
        int (*compare)(const void *, const void *);
    } comparator;
};

/* 8. Function pointer with nested struct parameter */
typedef int (*processor_t)(struct { int a; int b; } param);

/* 9. Array of pointers to functions returning pointers to arrays */
int (*(*func_array[3])(int))[5];

/* 10. Typedef chain with all delimiter types */
typedef struct node {
    struct node *next;
    int data[(sizeof(void *) * 2)];
    void (*action)(struct node *);
} node_t;

#endif /* FILE1_H */
