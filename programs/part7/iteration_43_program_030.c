/* complex-types.h - Header with complex type declarations to test gengtype parsing */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*simple_fp)(int, int);
typedef void (*complex_fp1)(int (*)(char), double);
typedef int (*(*complex_fp2)(int (*)(char), double))(float);
typedef void (*(*signal_proto)(int, void (*)(int)))(int);

/* Test 2: Multi-dimensional arrays with complex size expressions */
#define ARRAY_SIZE(x) ((x) > 0 ? (x) : 1)
extern int multi_dim_array[10][ARRAY_SIZE(5)][(sizeof(int) * 8)];

/* Test 3: Struct with flexible array member and nested struct */
struct outer_struct {
    int id;
    struct {
        int x;
        int y;
        int (*calc)(struct outer_struct *, int);
    } inner;
    int data[];
};

/* Test 4: Union with function pointer array */
union complex_union {
    int (*func_array[5])(int, int);
    struct {
        int count;
        int (*nested_func)(int (*)(int), int);
    } u_data;
};

/* Test 5: Typedef combining function pointer returning array pointer */
typedef int (*(*callback_type)(void))[10];
typedef void (*(*(*deeply_nested_fp)(int (*)(int)))[5])(void);

/* Test 6: Struct containing array of function pointers */
struct operations {
    const char *name;
    int (*ops[5])(int, int);
    struct operations *(*factory)(int, const char *);
};

/* Test 7: Macro generating complex types */
#define DECLARE_COMPLEX_FP(n) int (*(*fp##n)(int))[n]
#define CREATE_NESTED_TYPE(t) typedef t (*(*nested_##t##_ptr)(t))(*)

/* Test 8: Type with all three delimiters deeply nested */
typedef struct {
    int matrix[3][(2 + 1)];
    union {
        int (*func)(int[2], struct { int x; int y; });
        void (*action)(void (*)(void), int);
    } u;
    int (*(*get_matrix)[3][2])(void);
} ultra_complex_t;

/* Test 9: Function prototype with complex parameter */
extern void register_callback(int (*cb)(int (*)(int), int[][2]), void *data);

/* Test 10: Variable declaration with nested sizeof */
extern size_t computed_size;

#endif /* COMPLEX_TYPES_H */
