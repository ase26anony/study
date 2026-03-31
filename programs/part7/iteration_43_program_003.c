/* { dg-do compile } */
/* Complex type definitions to test gengtype's consume_balanced function */

#ifndef FILE1_H
#define FILE1_H

/* 1. Function pointers with nested argument lists */
typedef int (*callback_t)(int (*)(char), double);
typedef void (*(*signal_handler_t)(int sig, void (*func)(int)))(int);

/* 2. Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(arg) ((arg) > 2 ? 5 : 3)
extern int multi_array[10][DYNAMIC_SIZE(4)];
typedef int matrix_t[5][(sizeof(int)*8)];

/* 3. Struct with flexible array member */
struct flexible_struct {
    int length;
    double data[];
};

/* 4. Complex nested struct with function pointer array */
struct nested_container {
    int id;
    struct {
        int (*operations[3])(struct nested_container*, int);
        union {
            int ival;
            void *ptr;
        } u;
    } inner;
};

/* 5. Function returning pointer to array */
typedef int (*(*array_returner_t)(void))[10];

/* 6. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_FP(n) int (*(*fp##n)(int))[n]
DECLARE_COMPLEX_FP(5);

/* 7. Deeply nested parentheses in function pointer */
typedef int (*(*(*deep_nested_fp)(int (*(*)(double))[3]))(char))[4];

/* 8. Struct with nested initializer-style type (for gengtype parsing) */
struct point3d {
    int x, y, z;
};

/* 9. Union with complex members */
union variant {
    int (*func_ptr)(int, int);
    struct {
        int count;
        int (*handlers[])(void);
    } dynamic;
};

#endif /* FILE1_H */
