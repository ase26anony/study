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
            int : 0; /* Force alignment */
        };
        long c;
    };
    int (*(*func_ptr_arr[2])(void))[3];
    struct {
        struct Inner {
            int x;
            int y;
        } nested;
        int z;
    } anonymous;
};

/* Function pointer array type */
typedef int (*array_of_5_funcs[5])(char, ...);

/* GCC attributes with nested parentheses */
#define ALIGNED_STRUCT __attribute__((aligned(32)))
#define FORMAT_PRINTF __attribute__((format(printf, 2, 3)))

/* Macro to generate complex types */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n])

/* Complex type with all delimiters mixed */
typedef struct {
    int (*func1)(int, int (*)(int[10], ...));
    union {
        int arr[2][3];
        struct {
            int a;
            int b;
        };
    } data;
} MixedDelimiters;

#endif /* COMPLEX_TYPES_H */
