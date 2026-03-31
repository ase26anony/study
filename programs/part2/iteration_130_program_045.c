#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer type with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];

/* Multi-dimensional array type */
typedef int matrix_t[3][4];

/* Nested structure with anonymous structs and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int c : 10;
        };
        struct {
            long d : 15;
            unsigned long e : 17;
        };
        long f;
    };
    int (*(*func_ptr_arr[2])(void))[3];
    char (*nested_array[5][7])[11];
};

/* Function pointer array type with variadic arguments */
typedef int (*array_of_5_funcs[5])(char, ...);

/* Deeply nested function pointer */
typedef void (*(*(**deep_nested)(int (*(*)[7])(double)))[8])(short);

/* GCC attributes with nested parentheses */
#define ALIGNED_ARRAY __attribute__((aligned(32)))
#define FORMAT_PRINTF __attribute__((format(printf, 2, 3)))

/* Macro to generate complex types */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*complex_array_##n)[n])(char (*)[n][2*n])

/* Conditional compilation for different delimiter patterns */
#ifdef USE_EXTREME_NESTING
    #define NESTED_PARENS(x) (((((x))))))
    #define NESTED_BRACES {{{{}}}}
    #define NESTED_BRACKETS [[[[[[[]]]]]]]
#else
    #define NESTED_PARENS(x) (x)
    #define NESTED_BRACES {}
    #define NESTED_BRACKETS []
#endif

#endif /* COMPLEX_TYPES_H */
