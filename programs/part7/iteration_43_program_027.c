/* complex-types.h - Header with deeply nested delimiter patterns for gengtype coverage */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists */
typedef int (*simple_fp)(int, char);
typedef int (*complex_fp)(int (*)(char), double);
typedef void (*(*signal_proto)(int sig, void (*handler)(int)))(int);

/* 2. Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 0 ? (x) : 1)
extern int multi_dim_array[10][DYNAMIC_SIZE(5)][(sizeof(int)*2)];

/* 3. Struct with flexible array member and nested function pointer */
struct container {
    int len;
    int data[];
};

struct operations {
    int (*math_ops[3])(int, int);
    void (*io_ops)(struct container *);
};

/* 4. Deeply nested typedef combining all delimiter types */
typedef int (*(*(*deep_nested)(int (*(*)(int))[5]))(void))[10];

/* 5. Union with nested struct initializer pattern */
union variant {
    struct {
        int type;
        union {
            int i;
            double d;
            void *p;
        } value;
    } tagged;
    long long raw;
};

/* 6. Macro generating complex types with parentheses */
#define DECLARE_CALLBACK(n) int (*(*callback##n)(int (*(*)(int))[n]))[n*2]

/* 7. Struct containing array of function pointers returning pointers to arrays */
struct processor {
    int (*(*handlers[5])(int))[10];
    void (*cleanup)(struct processor *);
};

/* 8. Type with nested parentheses in sizeof context */
typedef char buffer_t[sizeof(struct { int a; double b; }) > 16 ? 128 : 64];

/* 9. Function prototype with complex return type */
struct operations *get_operations(void);
int (*(*get_matrix_handler(int idx))(int))[10];

/* 10. Nested struct definition with bitfields */
struct outer {
    struct {
        unsigned int flag1 : 1;
        unsigned int flag2 : 2;
        struct {
            int x, y;
        } point;
    } inner;
    int data[4];
};

#endif /* COMPLEX_TYPES_H */
