/* complex-types.h - Header with deeply nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*simple_fp)(int, char);
typedef void (*(*complex_fp)(int (*)(char), double))(float);
typedef int (*(*signal_handler)(int sig, void (*func)(int)))(int);

/* Test 2: Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x > 0) ? x : 1)
extern int multi_dim_array[10][DYNAMIC_SIZE(5)][(sizeof(int)*2)];

/* Test 3: Struct with nested initializer-style declaration */
struct NestedStruct {
    int (*compare)(struct NestedStruct *, struct NestedStruct *);
    union {
        int ival;
        float fval;
        void *pval;
    } data[2][3];
    struct {
        int x;
        int y;
        int z;
    } coords;
};

/* Test 4: Function returning pointer to array of function pointers */
typedef int (*(*get_operations(void))[5])(int, int);

/* Test 5: Deeply nested parentheses in type casts */
typedef char *(*string_processor)(int (*(*)(void))[10]);

/* Test 6: Variable-length array in struct (flexible array member) */
struct Container {
    int count;
    int items[];
};

/* Test 7: Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_FP(n) int (*(*fp##n)(int (*(*)(int))[n]))[n]
DECLARE_COMPLEX_FP(3);
DECLARE_COMPLEX_FP(5);

/* Test 8: Nested struct with bitfields and arrays */
struct Outer {
    struct Inner {
        unsigned int flags : 3;
        signed int value : 5;
        char name[10];
    } inner_array[2];
    void (*methods[3])(struct Inner *);
};

/* Test 9: Union with anonymous struct containing function pointers */
union ComplexUnion {
    struct {
        int (*init)(void);
        void (*cleanup)(int (*)(void));
    } ops;
    long long data;
};

/* Test 10: Typedef chain with increasing complexity */
typedef int basic_t;
typedef basic_t (*fp_basic_t)(basic_t);
typedef fp_basic_t (*fp_fp_basic_t)(fp_basic_t, basic_t (*)(void));

#endif /* COMPLEX_TYPES_H */
