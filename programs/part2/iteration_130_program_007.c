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
    struct {
        int (*nested_func)(char (*)[5], ...);
    } inner;
};

/* Array of function pointers with variable arguments */
typedef int (*array_of_5_funcs[5])(char, ...);

/* Deeply nested type with all delimiter types */
typedef struct {
    int (*(*(*deep_nested)[5])(int (*)[3]))[2];
    union {
        struct {
            char (*ch_array[4])[10];
        };
        void *ptr;
    } data;
} UltraComplexType;

/* GCC attributes with nested parentheses */
#define ALIGNED_TYPE __attribute__((aligned(32)))
#define FORMAT_PRINTF __attribute__((format(printf, 2, 3)))

/* Macro to generate complex types */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]); \
    typedef struct { \
        int arr[n][n+1]; \
        void (*funcs[n])(int (*)[n]); \
    } Type##n##_t

/* Function declarations with complex parameters */
FORMAT_PRINTF void log_message(const char *fmt, ...);
ALIGNED_TYPE void* allocate_aligned(size_t size);

#endif /* COMPLEX_TYPES_H */
