#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];
typedef void (*(*signal_handler)(int, void (*)(int)))(int);

/* Multi-dimensional array type with function pointers */
typedef int (*(*array_of_funcs[5])(char, ...))[3];

/* Nested structure with anonymous unions and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int c : 10;
        };
        struct {
            long d : 15;
            long e : 17;
        };
        unsigned long long f;
    };
    
    /* Array of function pointers returning pointers to arrays */
    int (*(*func_ptr_arr[2])(void))[3];
    
    /* Nested structure inside structure */
    struct Inner {
        int (*(*nested_func)(int[2][3]))(char (*)[4]);
        union {
            struct {
                short x : 4;
                short y : 4;
            };
            unsigned char z;
        } nested_union;
    } inner;
};

/* GCC attributes with nested parentheses */
typedef int __attribute__((aligned(32))) aligned_int;
typedef void (* __attribute__((format(printf, 2, 3))) printf_func)(void *, const char *, ...);

/* Macro to generate complex types with varying delimiter nesting */
#define MAKE_COMPLEX_TYPE(n) int (*(*var##n)[n])(char (*)[n])
#define MAKE_NESTED_ARRAY(n) int (*(*arr_ptr##n)[n][n])(void)

/* Variadic function pointer type */
typedef int (*(*variadic_func_ptr)(int, ...))(double, ...);

#endif /* COMPLEX_TYPES_H */
