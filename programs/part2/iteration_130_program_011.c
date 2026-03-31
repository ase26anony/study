/* complex_types.h */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Nested delimiter patterns in type declarations */
typedef int (*array_of_funcs[5])(char, ...);

/* Function pointer with nested parameter list */
typedef void (*(*complex_callback)(int (*)(float, double), ...))(void);

/* Structure with deeply nested anonymous structs and bit-fields */
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
    int (*(*func_ptr_arr[3])(void))[4];
    
    /* Nested structure with array of function pointers */
    struct Inner {
        char (*(*string_proc)(int, ...))[20];
        double matrix[2][3];
    } inner;
};

/* Macro to generate complex types with varying delimiter nesting */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n], int (*)(int[n][n]))

/* Attribute with nested parentheses */
#define FORMAT_ATTR __attribute__((format(printf, 2, 3)))
#define ALIGNED_ATTR __attribute__((aligned(32)))

#endif /* COMPLEX_TYPES_H */
