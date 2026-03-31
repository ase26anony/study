/* complex-types.h - Test file for gengtype consume_balanced coverage */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*simple_fp)(int, char);
typedef int (*complex_fp)(int (*)(char), double);
typedef void (*(*signal_proto)(int, void (*)(int)))(int);

/* Test 2: Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 0 ? (x) : 1)

struct Matrix {
    int rows;
    int cols;
    int data[DYNAMIC_SIZE(10)][DYNAMIC_SIZE(20)];
};

/* Test 3: Flexible array member in nested struct */
struct Outer {
    int id;
    struct {
        int len;
        int flexible[];
    } inner;
};

/* Test 4: Function returning pointer to array */
typedef int (*(*callback_factory)(void))[10];

/* Test 5: Array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*(*advanced[3])(int))[2];
};

/* Test 6: Deeply nested parentheses in macros */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]
#define NESTED_MACRO(x) sizeof(int[(x) + 1])

/* Test 7: Struct with bitfields and nested arrays */
struct BitfieldStruct {
    unsigned int flags : 3;
    unsigned int : 2; /* Padding */
    unsigned int count : 27;
    char name[(sizeof(int) * 8)];
};

/* Test 8: Union with anonymous struct containing array */
union DataContainer {
    struct {
        int type;
        union {
            int i;
            double d;
            char str[20];
        } value;
    } tagged;
    long long raw;
};

/* Test 9: Typedef chain with parentheses */
typedef int (*level1)(int);
typedef level1 (*level2)(char);
typedef level2 (*level3)(double);

/* Test 10: Variable declarations with casts in array bounds */
extern int (*global_table)[(sizeof(int)*8)];

#endif /* COMPLEX_TYPES_H */
