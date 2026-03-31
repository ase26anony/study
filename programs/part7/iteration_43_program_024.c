/* complex-types.h - Header with complex nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*simple_fp)(int, double);
typedef int (*complex_fp)(int (*)(char), double);
typedef void (*(*signal_proto)(int sig, void (*func)(int)))(int);

/* Test 2: Multi-dimensional arrays with nested size expressions */
typedef int matrix_2d[10][20];
typedef int vla_matrix[][(sizeof(int) > 4) ? 8 : 4];
extern int argc;  /* To make expressions non-constant for gengtype */

/* Test 3: Struct with flexible array member */
struct flexible_struct {
    int len;
    int data[];
};

/* Test 4: Struct containing array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*handlers[3])(void (*)(void));
};

/* Test 5: Deeply nested function pointer type */
typedef int (*(*(*deep_nested_fp)(int (*)(char)))[10])(double, float);

/* Test 6: Union with nested struct initializer pattern */
union nested_union {
    struct {
        int x;
        int y[3];
    } point;
    struct {
        void (*callback)(int, int);
        int matrix[2][2];
    } handler;
};

/* Test 7: Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_ARRAY(n) int (*(*fp_array##n[n])(int))[n]
#define CREATE_NESTED_PTR(n) void (*(*nested##n)(int (*(*)(void))[n]))(void)

/* Test 8: Typedef chain with nested delimiters */
typedef struct Node Node;
struct Node {
    Node *next;
    int (*compare)(Node *, Node *);
    int values[][3];
};

/* Test 9: Function returning pointer to array of function pointers */
typedef int (*(*get_operations(void))[10])(int, int);

/* Test 10: Struct with bitfield and nested array */
struct bitfield_struct {
    unsigned int flags : 3;
    int (*actions[4])(struct bitfield_struct *);
    union {
        int i;
        void (*fp)(void);
    } u;
};

#endif /* COMPLEX_TYPES_H */
