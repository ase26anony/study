/* complex-types.h - Header with deeply nested delimiter patterns for gengtype coverage */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*simple_fp)(int, char);
typedef void (*(*complex_fp)(int (*)(char), double))(float);
typedef int (*(*(*nested_fp)(void))(int, int(*)(int)))[10];

/* Test 2: Multi-dimensional arrays with complex size expressions */
#define DYNAMIC_SIZE(x) ((x) > 0 ? (x) : 1)

struct ArrayStruct {
    int matrix[10][(sizeof(int) * 8)];
    int vla[][DYNAMIC_SIZE(5)];
};

/* Test 3: Struct with flexible array member containing function pointers */
struct Container {
    int count;
    struct {
        int id;
        void (*action)(int, char);
    } items[];
};

/* Test 4: Union with nested struct and array initializers */
union NestedUnion {
    struct {
        int x;
        int y[3][(2 + 3)];
    } point;
    struct {
        void (*callback)(int (*)(char), double);
        int matrix[2][(4 - 1)];
    } handler;
};

/* Test 5: Typedef combining all delimiter types */
typedef struct {
    int (*(*get_array)(int size))[(size > 0) ? size : 1];
    void (*process)(int (*filter)(int), int array[][10]);
} ComplexType;

/* Test 6: Macro generating complex types with parentheses */
#define DECLARE_CALLBACK(n) \
    typedef int (*(*callback##n)(int arg##n))[n]; \
    callback##n init_callback##n(void (*setup)(int, int(*)(int)));

DECLARE_CALLBACK(5)
DECLARE_CALLBACK(10)

/* Test 7: Struct with array of function pointers */
struct Operations {
    enum { MAX_OPS = 8 } tag;
    union {
        int (*arithmetic[5])(int, int);
        void (*io_ops[(sizeof(void*) * 2)])(char*, int);
    } ops;
};

/* Test 8: Deeply nested type using all delimiters */
typedef int (*(*(*(*deep_nested)(struct {
    int depth;
    int bounds[3][(4 + 1)];
} config))(int, ...))[(config.depth > 0) ? config.depth : 1])(char);

/* Test 9: Function returning pointer to array of function pointers */
int (*(*get_operation_table(void))[10])(int, int);

/* Test 10: Variable declarations with complex types */
extern int (*(*global_handler)(int (*)(char), double))[(sizeof(int) + 2)];

#endif /* COMPLEX_TYPES_H */
