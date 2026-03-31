/* complex-types.h - Header with deeply nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*callback_t)(int (*)(char), double);
typedef void (*(*signal_handler_t)(int sig, void (*)(int)))(int);

/* Test 2: Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 10 ? 20 : 5)
extern int multi_array[10][DYNAMIC_SIZE(15)][(sizeof(int)*2)];

/* Test 3: Struct with flexible array member and nested initializers */
struct nested_container {
    int depth;
    struct {
        int x;
        int y[3];
    } point;
    int (*operations[5])(int, struct nested_container*);
};

/* Test 4: Union with anonymous struct and function pointer array */
union complex_union {
    struct {
        int (*compare)(const void*, const void*);
        void (*process)(int matrix[][(4+1)]);
    } funcs;
    long data[(1 << 3)];
};

/* Test 5: Typedef combining function pointer returning array pointer */
typedef int (*(*array_generator_t)(void))[10];

/* Test 6: Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_FP(n) int (*(*fp##n)(int (*(*)(void))[n]))[n]
DECLARE_COMPLEX_FP(5);

/* Test 7: Struct with bitfields and array of function pointers */
struct operations_table {
    unsigned int flags : 3;
    int (*math_ops[4])(int, int);
    struct operations_table* (*next_table)(int size[(flags+1)]);
};

/* Test 8: Nested typedef chain with all delimiter types */
typedef struct node node_t;
struct node {
    node_t* (*get_next)(void);
    int data[((sizeof(node_t*) > 4) ? 8 : 4)];
    struct {
        node_t* children[2];
    } nested;
};

/* Test 9: Function prototype with complex parameter */
extern void process_matrix(int (*matrix)[][(10*2)], 
                           void (*callback)(int, int (*)(void)));

/* Test 10: Variable declaration with cast in array size */
extern char buffer[((int)((double)3.14 * 2.0))];

#endif /* COMPLEX_TYPES_H */
