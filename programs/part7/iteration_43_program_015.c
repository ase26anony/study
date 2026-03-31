/* complex-types.h - Header with deeply nested delimiter patterns for gengtype coverage */

#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists */
typedef int (*simple_fp)(int, int);
typedef int (*complex_fp)(int (*)(char), double);
typedef void (*(*signal_fp)(int sig, void (*handler)(int)))(int);

/* 2. Multi-dimensional arrays with nested size expressions */
typedef int matrix_t[10][(sizeof(int) > 4) ? 5 : 3];
typedef char string_array[][(16 * 2)];

/* 3. Struct with flexible array member */
struct flexible_struct {
    int len;
    int data[];
};

/* 4. Union with nested struct */
union nested_union {
    struct {
        int x;
        int y;
    } point;
    struct {
        float r;
        float theta;
    } polar;
};

/* 5. Function pointer returning pointer to array */
typedef int (*(*callback_ret_array)(void))[10];

/* 6. Struct containing array of function pointers */
struct operations {
    int (*ops[5])(int, int);
    void (*cleanup)(void);
};

/* 7. Deeply nested function pointer type */
typedef int (*(*(*deep_nested_fp)(int (*(*)(double))[3]))(char))[4];

/* 8. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_ARRAY(n) int (*(*fp_array##n[n])(int))[n]
#define CREATE_NESTED_TYPE(t) typedef t (*(*nested_##t##_ptr)(t))[sizeof(t)]

/* 9. Using the macro to create complex types */
DECLARE_COMPLEX_ARRAY(5);
CREATE_NESTED_TYPE(int);

/* 10. Struct with bitfields and nested arrays */
struct bitfield_struct {
    unsigned int flags : 3;
    signed int value : 5;
    int array[2][(8 / 2)];
};

/* 11. Anonymous struct/union in union */
union anonymous_container {
    struct {
        int tag;
        union {
            int i;
            float f;
            void *p;
        } value;
    } tagged;
    long long raw;
};

/* 12. Function with _Noreturn specifier and complex return */
typedef _Noreturn void (*noreturn_fp)(int, ...);

/* 13. Aligned attribute with nested expression */
struct aligned_struct {
    int data;
} __attribute__((aligned((sizeof(long) * 2))));

/* 14. Forward declaration with pointer to incomplete type */
struct incomplete;
typedef struct incomplete *(*get_incomplete_fp)(void);

#endif /* COMPLEX_TYPES_H */
