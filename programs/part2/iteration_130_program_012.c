#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer with nested parentheses */
typedef int (*(*complex_callback_t)(int (*)(float)))[10];

/* Multi-dimensional array type with function pointer elements */
typedef void (*(*array_of_funcs_t[5])(char, ...))[3];

/* Nested structure with anonymous unions/structs */
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
        struct Inner {
            int x;
            double y;
        } inner;
        int z;
    } nested;
};

/* Macro to generate complex types with varying delimiter nesting */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]); \
    typedef struct { \
        int matrix[n][n*2]; \
        void (*funcs[n])(int (*)(int[n]), ...); \
    } ComplexStruct##n

/* GCC attributes with nested parentheses */
#define FORMAT_ATTR __attribute__((format(printf, 2, 3)))
#define ALIGNED_ATTR __attribute__((aligned(32)))
#define PACKED_ATTR __attribute__((packed))

/* Function declarations using complex types */
complex_callback_t init_callback(void);
void process_matrix(int matrix[][4], int rows);

#endif /* COMPLEX_TYPES_H */
