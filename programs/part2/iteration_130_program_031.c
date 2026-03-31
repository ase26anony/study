#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];
typedef void (*(*signal_handler)(int, void (*)(int)))(void);

/* Multi-dimensional array type with function pointers */
typedef int (*(*array_of_funcs[5])(char, ...))[3];

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
            long e : 17;
        };
        unsigned long long f;
    };
    
    /* Array of function pointers returning pointers to arrays */
    int (*(*func_ptr_arr[2])(void))[3];
    
    /* Nested structure with bit-fields */
    struct Inner {
        struct {
            int x : 4;
            int y : 4;
            int z : 8;
        } nested;
        double value;
    } inner;
};

/* Complex type with attributes and nested parentheses */
typedef int (*(* __attribute__((aligned(32))) aligned_func_ptr)(int, ...))[10]
    __attribute__((deprecated("Use new_type instead")));

/* Macro to generate complex types with varying delimiter nesting */
#define MAKE_COMPLEX_TYPE(n) \
    typedef int (*(*var##n)[n])(char (*)[n]); \
    typedef struct { \
        int (*(*arr##n[n])(void))[n]; \
        char (*str##n)[n][n]; \
    } ComplexStruct##n

/* Generate several complex types */
MAKE_COMPLEX_TYPE(2);
MAKE_COMPLEX_TYPE(3);
MAKE_COMPLEX_TYPE(5);

/* Function with complex parameter and return types */
complex_callback (*(*get_callback_table(void))[5])(int (*)(float));

#endif /* COMPLEX_TYPES_H */
