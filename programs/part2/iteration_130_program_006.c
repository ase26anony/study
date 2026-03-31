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
        long d;
    };
    int (*(*func_ptr_arr[2])(void))[3];
    char (*nested_array[2][3])[4];
};

/* Function pointer array type with variadic arguments */
typedef int (*array_of_5_funcs[5])(char, ...);

/* Macro to generate complex types with nested delimiters */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]); \
    struct Inner##n { \
        int arr[n][n+1]; \
        void (*funcs[n])(int (*)[n]); \
    };

/* Generate several complex types */
MAKE_COMPLEX_TYPE(2)
MAKE_COMPLEX_TYPE(3)
MAKE_COMPLEX_TYPE(5)

/* Attribute with deeply nested parentheses */
#ifdef __GNUC__
#define FORMAT_PRINTF __attribute__((format(printf, 2, 3)))
#define ALIGNED_TYPE __attribute__((aligned(32)))
#else
#define FORMAT_PRINTF
#define ALIGNED_TYPE
#endif

/* Function declaration with complex return type and attributes */
complex_callback FORMAT_PRINTF create_callback(const char *fmt, ...);

#endif /* COMPLEX_TYPES_H */
