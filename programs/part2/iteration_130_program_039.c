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
            int c : 10;
        };
        long d;
    };
    int (*(*func_ptr_arr[2])(void))[3];
    struct {
        char *name;
        int (*comparator)(const void *, const void *);
    } nested;
};

/* Array of function pointers with variadic arguments */
typedef int (*array_of_5_funcs[5])(char, ...);

/* Function pointer returning pointer to array */
typedef float (*(*signal_handler)(int, void*))[2][2];

/* GCC attributes with nested parentheses */
#define FORMAT_PRINTF __attribute__((format(printf, 2, 3)))
#define ALIGNED_TYPE __attribute__((aligned(32)))
#define PACKED_STRUCT __attribute__((packed))

/* Macro generating complex types with nested delimiters */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]); \
    typedef struct { \
        int data[n][n]; \
        void (*ops[n])(int (*)(int[n]), ...); \
    } ComplexStruct##n

/* Declare some complex types using the macro */
MAKE_COMPLEX_TYPE(3);
MAKE_COMPLEX_TYPE(5);

#endif /* COMPLEX_TYPES_H */
