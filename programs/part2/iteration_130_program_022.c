#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];
typedef void (*(**signal_handler)(int, void (*)(int)))(void);

/* Nested array types with function pointers */
typedef int (*(*array_of_funcs[5])(char, ...))[3];
typedef float (*(*matrix_processor)(double (*)[4][4]))[3][3];

/* Structure with deeply nested anonymous structs and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int : 0;  /* zero-width bit-field */
        };
        struct {
            long c : 12;
            long d : 20;
        };
        long e;
    };
    
    /* Array of function pointers returning pointers to arrays */
    int (*(*func_ptr_arr[2])(void))[3];
    
    /* Nested function pointer in struct */
    char (*(*(*nested_fp)(int (*)(char)))[5])(float);
};

/* Union with complex type members */
union ComplexUnion {
    struct {
        int (*(*fp_member)(void))[10];
        __attribute__((aligned(32))) double aligned_array[4][4];
    };
    struct {
        void (*(*void_fp_array[3])(int, ...));
        __attribute__((format(printf, 2, 3))) int (*printf_func)(void *, const char *, ...);
    };
};

/* Macro to generate complex types with varying sizes */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]); \
    typedef struct { \
        int matrix[n][n]; \
        void (*(*callbacks[n])(int, ...)); \
    } ComplexStruct##n

/* Use the macro to generate multiple complex types */
MAKE_COMPLEX_TYPE(2);
MAKE_COMPLEX_TYPE(4);
MAKE_COMPLEX_TYPE(8);

/* Function with complex parameter and return types */
__attribute__((always_inline)) 
static inline int (*(*register_callback(
    int (*(*callback)(int (*)(float)))[10],
    __attribute__((nonnull)) void **context
))[5])(void) {
    return (int (*(*)[5])(void))0;
}

#endif /* COMPLEX_TYPES_H */
