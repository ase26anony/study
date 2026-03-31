/* complex-types.h - Header with deeply nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*simple_fp)(int, char);
typedef void (*(*complex_fp)(int (*)(char), double))(float);
typedef int (*(*signal_handler)(int sig, void (*func)(int)))(int);

/* Test 2: Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 10 ? 20 : 5)

struct ArrayStruct {
    int matrix[10][DYNAMIC_SIZE(15)];
    int (*ptr_matrix)[(sizeof(int) * 8)][DYNAMIC_SIZE(3)];
};

/* Test 3: Nested struct with flexible array member */
struct NestedContainer {
    struct {
        int len;
        int data[];
    } inner;
    struct ArrayStruct *next;
};

/* Test 4: Function returning pointer to array */
typedef int (*(*Callback)(void))[10];

/* Test 5: Struct containing array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*(*advanced_ops[3])(struct Operations*))[2];
};

/* Test 6: Macro generating complex types with parentheses */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]
DECLARE_COMPLEX(5);
DECLARE_COMPLEX(10);

/* Test 7: Union with nested initializer-style type */
union ComplexUnion {
    struct {
        int (*func_ptr)(int[((2+3)*2)]);
        char data[sizeof(int[(4+1)])];
    } s;
    double d;
};

/* Test 8: Typedef chain with nested delimiters */
typedef struct Node {
    struct Node *(*get_next)(void);
    void (*process)(int (*)(struct Node*));
} Node;

/* Test 9: Function prototype with all delimiter types */
extern void process_all(
    int (*callback)(int[][(2+3)], struct { int x; int y; }),
    struct Operations (*get_ops)(void),
    union ComplexUnion (*transform)(int (*)(char))
);

/* Test 10: Variable declaration with cast in array size */
extern int global_table[(int)((double)3.14 * 2)];

#endif /* COMPLEX_TYPES_H */
