#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];

/* Multi-dimensional array type */
typedef int (*(*array_of_func_ptrs[5])(void))[3];

/* Nested attribute with parentheses */
#define FORMAT_ATTR __attribute__((format(printf, 2, 3)))
#define ALIGNED_ATTR __attribute__((aligned(32)))

/* Macro to generate complex types with varying nesting */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n])

/* Structure with deeply nested delimiters */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int c : sizeof(int[2][3]); /* sizeof with nested brackets */
        };
        long d;
    };
    
    /* Array of function pointers returning pointers to arrays */
    int (*(*func_ptr_arr[2])(void))[3];
    
    /* Nested anonymous struct with bit-fields */
    struct {
        struct {
            unsigned x : sizeof(struct { int a; double b; });
            unsigned y : 7;
        } nested;
        float z;
    } inner;
};

/* Function pointer with variable arguments and attributes */
typedef int (*printf_like_func)(const char *, ...) FORMAT_ATTR;

/* Union with nested arrays and function pointers */
union ComplexUnion {
    struct {
        int (*(*callback)(int, ...))(double);
        char (*string_array[4])[20];
    } s;
    void *(*void_funcs[3])(struct Outer *);
};

#endif /* COMPLEX_TYPES_H */
