/* complex_types.h */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Nested attribute with parentheses */
#define FORMAT_ATTR __attribute__((format(printf, 2, 3)))
#define ALIGNED_ATTR(n) __attribute__((aligned(n)))

/* Macro generating complex types with nested delimiters */
#define MAKE_COMPLEX_PTR(n) int (*(*complex_ptr##n)[n])(char (*)[n], ...)
#define MAKE_NESTED_ARRAY(n) int (*(*nested_arr##n)[n][n])(void)

/* Complex structure with deeply nested anonymous members */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int c : 10;
        } bits;
        struct {
            long x;
            double y;
        } data;
    } inner_union;
    
    /* Function pointer array with nested signature */
    int (*(*func_ptr_arr[3])(int (*)(float, double), ...))[2];
    
    /* Nested array in parameter */
    void (*callback)(int matrix[2][3][4], ...);
};

/* Typedef with complex grouping */
typedef int (*array_of_funcs[5])(char, ...);
typedef int (*(*nested_func_ptr)(int (*)(float)))[10];

/* Structure with bitfields and function pointers */
struct Container {
    unsigned int flags : 8;
    unsigned int mode : 4;
    
    /* Pointer to array of function pointers */
    int (*(*(*deep_ptr)[2])(void))[3];
    
    /* Anonymous struct inside union */
    union {
        struct {
            short s;
            char c;
        };
        int i;
    } anon;
};

#endif /* COMPLEX_TYPES_H */
