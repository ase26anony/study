#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];

/* Multi-dimensional array type */
typedef int matrix_t[3][4];

/* Nested structure with anonymous structs and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
        };
        long c;
    };
    int (*(*func_ptr_arr[2])(void))[3];
};

/* Array of function pointers with variadic arguments */
typedef int (*array_of_5_funcs[5])(char, ...);

/* Macro to generate complex types with nested delimiters */
#define MAKE_COMPLEX_TYPE(n) int (*(*var##n)[n])(char (*)[n])

/* GCC attributes with nested parentheses */
#define PRINTF_FORMAT __attribute__((format(printf, 2, 3)))
#define ALIGNED_TYPE __attribute__((aligned(32)))

#endif /* COMPLEX_TYPES_H */
