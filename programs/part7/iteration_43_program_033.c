/* { dg-do compile } */
/* Primary header with complex nested delimiter patterns */

#ifndef FILE1_H
#define FILE1_H

/* 1. Function pointers with nested argument lists */
typedef int (*callback_t)(int (*)(char), double);
typedef void (*(*signal_handler_t)(int sig, void (*func)(int)))(int);

/* 2. Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(n) ((n) > 0 ? (n) : 1)
extern int multi_dim_array[10][DYNAMIC_SIZE(5)][(sizeof(int)*8)];

/* 3. Struct with flexible array member and nested initializers */
struct container {
    int len;
    int data[];
};

/* 4. Combined: function pointer returning pointer to array */
typedef int (*(*array_factory_t)(void))[10];

/* 5. Struct containing array of function pointers */
struct operations {
    int (*ops[5])(int, int);
    callback_t (*handlers[3])(void);
};

/* 6. Deeply nested parentheses in function pointer */
typedef int (*(*(*deep_nested_fp)(int (*(*)(double))[3]))(char))[5];

/* 7. Union with nested struct initializer */
union complex_union {
    struct {
        int (*func_ptr)(int);
        int matrix[2][(4 + 1)];
    } nested;
    long long raw;
};

/* 8. Macro expanding to complex type with parentheses */
#define DECLARE_COMPLEX_TYPE(n) \
    typedef int (*(*complex_fp##n)(int (*(*)(double))[n]))[n * 2]

DECLARE_COMPLEX_TYPE(3);
DECLARE_COMPLEX_TYPE(5);

/* 9. Variable-length array in struct */
struct vla_container {
    int size;
    int arr[(sizeof(void*) * 2)];
};

/* 10. Nested anonymous struct/union */
struct outer {
    struct {
        int (*compare)(const void*, const void*);
        union {
            int (*int_func)(int);
            void (*void_func)(void);
        } func_union;
    } inner;
    int data;
};

#endif /* FILE1_H */
