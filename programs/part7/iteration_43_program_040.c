/* complex-types.h - Header with complex nested delimiter patterns for gengtype coverage */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists */
typedef int (*simple_fp)(int, double);
typedef void (*(*complex_fp_ptr)(int (*)(char), double))(float);
typedef int (*(*nested_fp)(int (*(*)(void))[5]))(void);

/* 2. Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 0 ? (x) : 1)

struct ArrayStruct {
    int matrix[10][DYNAMIC_SIZE(5)];
    int (*vla)[DYNAMIC_SIZE(3)];
};

/* 3. Struct with nested initializer-style member */
struct NestedAggregate {
    struct {
        int x;
        int y[2][2];
    } inner;
    int (*func_table[3])(int, int);
};

/* 4. Combined: function pointer returning pointer to array */
typedef int (*(*callback_func)(void))[10];

/* 5. Deeply nested parentheses for function pointers */
void (*(*signal_handler(int sig, void (*(*handler)(int))(int)))(int));

/* 6. Union with array of function pointers */
union OperationUnion {
    int (*math_ops[4])(int, int);
    float (*float_ops[2])(float, float);
};

/* 7. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_FP(n) int (*(*fp##n)(int (*(*arg)(void))[n]))[n]
DECLARE_COMPLEX_FP(5);

/* 8. Struct containing flexible array member */
struct FlexibleContainer {
    int count;
    int data[];
};

/* 9. Typedef chain with nested delimiters */
typedef struct Node Node;
struct Node {
    Node *next;
    int (*(*get_array)(void))[5];
};

/* 10. Complex cast expression type */
typedef int *(*(*cast_example)(long (*(*)(double))[3]))[2];

/* External declarations for multi-file testing */
extern int (*(*external_fp)(int (*(*)(char))[3]))[4];
extern struct ArrayStruct external_array;

#endif /* COMPLEX_TYPES_H */
