/* complex-types.h - Primary header with deeply nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*simple_fp)(int, char);
typedef int (*complex_fp)(int (*)(char), double);
typedef void (*(*signal_fp)(int sig, void (*handler)(int)))(int);

/* Test 2: Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 0 ? (x) : 1)

struct ArrayTest {
    int matrix[10][DYNAMIC_SIZE(5)];
    int (*vla)[DYNAMIC_SIZE(sizeof(int) * 8)];
};

/* Test 3: Nested structs with complex initializers */
struct Point3D {
    int x;
    struct {
        int y, z;
    } coord;
};

struct Container {
    struct Point3D points[4];
    int (*operations[3])(struct Point3D*, int);
};

/* Test 4: Function returning pointer to array */
typedef int (*(*Callback)(void))[10];

/* Test 5: Deeply nested combination */
typedef int (*(*(*deep_nested)(int (*(*)(int[][3]))(double)))[5])(char);

/* Test 6: Struct with array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*(*signal_handlers[3]))(int);
};

/* Test 7: Union with nested anonymous struct */
union MixedData {
    int i;
    struct {
        float f;
        char str[(sizeof(int) * 2)];
    } data;
    void (*func_ptr)(union MixedData*);
};

/* Test 8: Macro generating complex types */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]
DECLARE_COMPLEX(5);
DECLARE_COMPLEX((2 + 3));

/* Test 9: Typedef chain with parentheses */
typedef struct Node Node;
typedef Node* (*NodeAllocator)(Node**, int);
typedef int (*(*NodeProcessor)(NodeAllocator))(void);

struct Node {
    Node* next;
    Node* prev;
    void* data;
    int (*compare)(Node*, Node*);
};

/* Test 10: Complex cast expressions type */
typedef int* (*CastFunc)(void*);
typedef void* (*(*DoubleCast)(CastFunc))(int**);

#endif /* COMPLEX_TYPES_H */
