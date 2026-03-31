#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer type with nested parentheses */
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
        int (*nested_func)(char (*)[5], ...);
        int arr[2][3][4];
    } inner;
};

/* Type definition with array of function pointers */
typedef int (*array_of_5_funcs[5])(char, ...);

/* Function pointer returning pointer to array */
typedef int (*(*signal_handler)(int, void*))[2];

/* GCC attributes with nested parentheses */
#define FORMAT_ATTR __attribute__((format(printf, 2, 3)))
#define ALIGNED_ATTR __attribute__((aligned(32)))
#define PACKED_ATTR __attribute__((packed))

/* Macro to generate complex types */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*complex_array_##n)[n])(char (*)[n]); \
    struct Complex##n { \
        int (*callbacks[n])(int (*)(float[n][n])); \
        int matrix[n][n]; \
    };

/* Declare some complex types using the macro */
MAKE_COMPLEX_TYPE(3)
MAKE_COMPLEX_TYPE(5)

/* Variable declarations with attributes */
extern complex_callback global_cb FORMAT_ATTR;
extern struct Outer outer_instance ALIGNED_ATTR;

#endif /* COMPLEX_TYPES_H */
