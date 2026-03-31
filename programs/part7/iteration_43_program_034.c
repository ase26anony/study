/* complex_types.h - Header with deeply nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*simple_fp)(int, double);
typedef void (*(*complex_fp)(int (*)(char), double))(float);
typedef int (*(*nested_fp)(int (*(*)(void))[5]))(void);

/* Test 2: Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 0 ? (x) : 1)

struct ArrayStruct {
    int matrix[10][(sizeof(int) * 8)];
    int vla[((DYNAMIC_SIZE(5)) + 2)];
    int (*ptr_array)[(DYNAMIC_SIZE(3))];
};

/* Test 3: Struct with flexible array member and nested struct */
struct NestedContainer {
    struct Inner {
        int (*callback)(int, int);
        int values[3];
    } inner;
    int data[];
};

/* Test 4: Union with anonymous struct containing function pointer array */
union ComplexUnion {
    struct {
        int (*(*func_ptrs[3])(int))[2];
        char tag;
    };
    long long as_ll;
};

/* Test 5: Typedef combining all delimiter types */
typedef struct {
    int (*(*get_matrix)(int rows))[(rows > 0) ? rows : 1];
    void (*processor)(int (*)(int), int[]);
} MatrixProcessor;

/* Test 6: Macro that expands to complex type with parentheses */
#define DECLARE_CALLBACK(n) \
    typedef int (*(*callback##n)(int (*(*)(void))[n]))[(n)]

DECLARE_CALLBACK(2);
DECLARE_CALLBACK(3);
DECLARE_CALLBACK(4);

/* Test 7: Struct with bitfield and array of function pointers */
struct Operations {
    unsigned int flags : ((sizeof(int) * 8) - 1);
    int (*ops[((5) + (2))])(int, int);
    struct Operations *next;
};

/* Test 8: Type with cast-like expressions in array bounds */
typedef int array_with_cast[(int)((double)3.14)];

/* Test 9: Deeply nested parentheses in function pointer */
typedef void (*(*(*deep_nested)(int (*(*)(int (*(*)(void))[3]))(void)))(int))[2];

#endif /* COMPLEX_TYPES_H */
