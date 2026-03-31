#ifndef TEST_COMPLEX_TYPES_H
#define TEST_COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];
typedef void (*(*signal_handler)(int, void (*)(int)))(void);

/* Multi-dimensional array type with function pointers */
typedef int (*(*array_of_func_ptrs[5][3])(char, ...))[2];

/* Nested structure with anonymous unions/structs */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int c : 10;
        };
        struct {
            long d : 12;
            long e : 20;
        };
        unsigned long long f;
    };
    
    /* Array of function pointers returning pointers to arrays */
    int (*(*func_ptr_arr[2])(void))[3];
    
    /* Nested function pointer in struct */
    char (*(*(*nested_fp)(int (*)(float[2][3])))[4])(double);
};

/* GCC attributes with nested parentheses */
#define ALIGNED_TYPE __attribute__((aligned(32)))
#define FORMAT_PRINTF __attribute__((format(printf, 2, 3)))

/* Macro to generate complex types with varying delimiter depths */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]); \
    typedef struct { \
        int matrix[n][n*2]; \
        void (*(*callbacks[n])(int, ...))(); \
    } ComplexStruct##n;

/* Variadic function pointer type */
typedef int (*variadic_func)(int, ...);

#endif /* TEST_COMPLEX_TYPES_H */
