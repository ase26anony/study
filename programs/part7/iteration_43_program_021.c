/* complex-types.h - Header with deeply nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists */
typedef int (*simple_fp)(int, double);
typedef void (*(*complex_fp)(int (*)(char), double))(float);
typedef int (*(*nested_fp)(int (*(*)(void))[5]))(void);

/* 2. Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 0 ? (x) : 1)

struct ArrayStruct {
    int matrix[10][DYNAMIC_SIZE(5)];
    int (*vla)[DYNAMIC_SIZE(3)];
};

/* 3. Struct with flexible array member */
struct Flexible {
    int len;
    int data[];
};

/* 4. Union with nested struct initializer pattern */
union NestedUnion {
    struct {
        int (*callback)(int, int);
        int matrix[2][(sizeof(int) + 3)];
    } s;
    double d;
};

/* 5. Function pointer returning pointer to array */
typedef int (*(*Callback)(void))[10];

/* 6. Struct containing array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*(*signal[3])(int, void (*)(int)))(int);
};

/* 7. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]
#define CREATE_NESTED(n) void (*(*nest##n)(int (*(*)(int))[n]))(void)

/* 8. Deeply nested typedef chain */
typedef struct Node Node;
struct Node {
    Node *next;
    int (*(*get_array)(Node *))[(sizeof(Node) + 7) / 8];
    void (*process)(int (*)(Node *, int), double);
};

/* 9. Type with all three delimiters deeply nested */
typedef struct {
    int (*funcs[3])(int, int (*)(char));
    struct {
        int matrix[2][(sizeof(int)*2)];
        union {
            int i;
            double d;
        } u;
    } nested;
} TripleNested;

/* 10. Function prototypes using complex types */
complex_fp create_complex(int (*)(char), double);
int process_operations(struct Operations *ops, int (*(*)(void))[10]);
Callback get_callback(void (*(*)(int))(float));

/* 11. Inline function with nested cast expression */
static inline int complex_cast(void *ptr) {
    return (int)((double*)((char*)ptr + sizeof(int)))[0];
}

/* 12. sizeof/alignof expressions in type declarations */
struct SizedStruct {
    size_t sz;
    int arr[sizeof(struct { int a; double b; }) / sizeof(int)];
    int aligned[__alignof__(double)];
};

#endif /* COMPLEX_TYPES_H */
