#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer type with nested parentheses */
typedef int (*(*complex_callback_t)(int (*)(float)))[10];

/* Multi-dimensional array type */
typedef int matrix_t[3][4];

/* Nested structure with anonymous structs and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int c : 8;
        };
        struct {
            long d : 12;
            long e : 20;
        };
        unsigned long long f;
    };
    int (*(*func_ptr_arr[2])(void))[3];
    complex_callback_t callback;
};

/* Function pointer array type with variadic arguments */
typedef int (*array_of_5_funcs[5])(char, ...);

/* Deeply nested array/function pointer combination */
typedef void (*(*(*deep_nested_t)[5])(int (*(*)(char [][10]))[3]))(double);

/* GCC attributes with nested parentheses */
#define ALIGNED_TYPE __attribute__((aligned(32)))
#define FORMAT_PRINTF __attribute__((format(printf, 2, 3)))

/* Macro to generate complex types with parameterized nesting */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*complex_array_##n)[n])(char (*)[n]); \
    struct Nested##n { \
        int (*(*arr_ptr##n)[n])(void); \
        char (*str_array##n)[n][n+1]; \
    };

/* Generate several complex types */
MAKE_COMPLEX_TYPE(2)
MAKE_COMPLEX_TYPE(3)
MAKE_COMPLEX_TYPE(5)

#endif /* COMPLEX_TYPES_H */
