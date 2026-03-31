#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];
typedef void (*(**signal_handler_ptr)(int, ...))(void);

/* Multi-dimensional array type with function pointer elements */
typedef int (*(*array_of_funcs[5])(char, ...))[3];

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
    
    /* Nested structure with bit-fields */
    struct Inner {
        int (*(*nested_func)(int[2][3]))(char (*)[4]);
        struct {
            int x : 8;
            int y : 8;
            int z : 16;
        } bits;
    } inner;
};

/* Macro to generate complex types with nested delimiters */
#define MAKE_COMPLEX_TYPE(n) int (*(*var##n)[n])(char (*)[n])
#define NESTED_ARRAY_TYPE(dim1, dim2) int (*(*nested_array_##dim1##_##dim2)[dim1][dim2])(void)

/* GCC attributes with nested parentheses */
#define FORMAT_PRINTF __attribute__((format(printf, 2, 3)))
#define ALIGNED_TYPE __attribute__((aligned(32)))
#define PACKED_STRUCT __attribute__((packed))

#endif /* COMPLEX_TYPES_H */
