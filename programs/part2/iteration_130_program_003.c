#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];
typedef void (*(*signal_handler)(int, void (*)(int)))(int);

/* Nested array types with function pointers */
typedef int (*(*array_of_funcs[5])(void))[3];
typedef char (*(*(*nested_func_arr[2][3])(int))[4])(double);

/* Structure with deeply nested delimiters */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int c : 10;
        };
        long d;
        struct {
            float e;
            double f;
        };
    };
    
    /* Function pointer array member */
    int (*(*func_ptr_arr[2])(void))[3];
    
    /* Nested array with initializer-style declaration */
    int matrix[3][4];
};

/* Union with anonymous struct and bit-fields */
union Container {
    struct {
        unsigned int flags : 8;
        unsigned int mode : 4;
        unsigned int : 4;  /* Padding */
        unsigned int count : 16;
    };
    unsigned int raw;
    struct {
        char *name;
        void (*callback)(int, ...);
    } meta;
};

/* Macro to generate complex types with varying delimiter nesting */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]); \
    typedef struct { \
        int (*(*arr##n[n])(int (*)[n]))[n]; \
        union { \
            struct { int x : n; int y : (32-n); }; \
            long all; \
        } bits; \
    } ComplexStruct##n

/* Variadic function pointer type */
typedef int (*printf_like_func)(const char *, ...);

/* Attribute specifications with nested parentheses */
#define ALIGNED_TYPE __attribute__((aligned(32)))
#define FORMAT_ATTR __attribute__((format(printf, 2, 3)))

#endif /* COMPLEX_TYPES_H */
