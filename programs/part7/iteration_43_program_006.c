/* complex-types.h - Header with deeply nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*simple_fp)(int, char);
typedef int (*complex_fp)(int (*)(char), double);
typedef void (*(*signal_fp)(int sig, void (*func)(int)))(int);

/* Test 2: Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 0 ? (x) : 1)

struct ArrayStruct {
    int matrix[10][DYNAMIC_SIZE(5)];
    int (*ptr_matrix)[(sizeof(int) * 8)];
};

/* Test 3: Nested structs with complex initializers */
struct Point {
    int x;
    int y;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
    int (*area_calc)(struct Rectangle *);
};

/* Test 4: Function returning pointer to array */
typedef int (*(*Callback)(void))[10];

/* Test 5: Struct containing array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*(*complex_ops[3])(struct Operations *))[2];
};

/* Test 6: Macro expanding to complex type with parentheses */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]

/* Test 7: Union with nested struct and array */
union NestedUnion {
    struct {
        int data;
        int (*processor)(int, int (*)(int));
    } s;
    int arr[((sizeof(int) + 3) & ~3)];
};

/* Test 8: Typedef chain with multiple parentheses */
typedef struct Node Node;
typedef Node *(*(*NodeProcessor)(Node **, int))(void);

struct Node {
    Node *next;
    Node *prev;
    void *data;
    int (*compare)(Node *, Node *);
};

/* Test 9: Variable-length array in struct */
struct Flexible {
    int len;
    int arr[];  /* Flexible array member */
};

/* Test 10: Complex cast expression type */
typedef int *(*(*CastExample)(long, double))[(1 << 3)];

/* External declarations */
extern struct Operations global_ops;
extern Callback get_callback(void);
extern void register_signal(signal_fp handler);

#endif /* COMPLEX_TYPES_H */
