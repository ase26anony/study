/* complex-types.h - Header with deeply nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*simple_fp)(int, char);
typedef int (*complex_fp)(int (*)(char), double);
typedef void (*(*signal_fp)(int sig, void (*func)(int)))(int);

/* Test 2: Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 10 ? 20 : 5)

struct ArrayTest {
    int matrix[10][(sizeof(int)*2)];
    int vla[][DYNAMIC_SIZE(15)];
};

/* Test 3: Struct with flexible array member */
struct Flexible {
    int len;
    int data[];
};

/* Test 4: Function returning pointer to array */
typedef int (*(*Callback)(void))[10];

/* Test 5: Struct containing array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*(*complex_ops[3])(char))[2];
};

/* Test 6: Deeply nested typedef chain */
typedef struct Node Node;
struct Node {
    int (*(*get_callback)(Node *n))(int);
    Node *children[(sizeof(Node*) * 4)];
};

/* Test 7: Union with nested struct initializer pattern */
union NestedUnion {
    struct {
        int (*func_ptr)(int[((2+3)*2)]);
        char arr[3][4];
    } inner;
    long long data;
};

/* Test 8: Macro generating complex types */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]
#define CREATE_ARRAY_TYPE(dim) int array_##dim[dim][(dim*2)][((dim+1)/2)]

/* Test 9: Type with all three delimiters deeply nested */
typedef struct {
    int (*methods[2])(
        struct {
            int x;
            int y[3];
        } point
    );
    union {
        int a;
        struct {
            char c;
            double d;
        } nested;
    } value;
} AllDelimiters;

/* Test 10: Function prototype with complex parameters */
extern void process_data(
    int (*(*callback)(int, void (*)(char)))[10],
    struct ArrayTest (*arrays[])[2],
    ...  /* Variadic to add more complexity */
);

/* Test 11: Inline function with nested cast expressions */
static inline int complex_cast(void *ptr) {
    return (int)((double*)((char*)ptr + sizeof(int)))[0];
}

/* Test 12: sizeof/alignof expressions with nested arrays */
#define COMPLEX_SIZE sizeof(int[(sizeof(char)*8)])
#define ALIGNED_SIZE _Alignof(struct { char c; int i[(2+3)]; })

#endif /* COMPLEX_TYPES_H */
