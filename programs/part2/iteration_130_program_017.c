#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];

/* Multi-dimensional array type */
typedef int (*(*array_of_func_ptrs[5])(void))[3];

/* Structure with nested anonymous structs and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int c : 10;
        };
        long d;
        struct {
            unsigned int e : 7;
            unsigned int f : 9;
        };
    };
    int (*(*func_ptr_arr[2])(void))[3];
    complex_callback cb;
};

/* Function pointer with variable arguments */
typedef int (*varargs_func)(char, ...);

/* Array of function pointers with complex signatures */
typedef varargs_func (*array_of_5_funcs[5])(char, ...);

/* Nested pointer to array of function pointers */
typedef int (*(*(**nested_ppfunc)[5])(int, ...))[10];

/* GCC attributes with nested parentheses */
#define ALIGNED_STRUCT __attribute__((aligned(32)))
#define FORMAT_FUNC __attribute__((format(printf, 2, 3)))

/* Macro to generate complex types with varying sizes */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]); \
    typedef struct { \
        int matrix[n][n*2]; \
        void (*funcs[n])(int (*)[n]); \
    } ComplexStruct##n;

/* Generate several complex types */
MAKE_COMPLEX_TYPE(2)
MAKE_COMPLEX_TYPE(3)
MAKE_COMPLEX_TYPE(5)

#endif /* COMPLEX_TYPES_H */
