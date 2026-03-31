#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];

/* Multi-dimensional array type with function pointer elements */
typedef void (*(*array_of_funcs[5])(char, ...))[3];

/* Nested structure with anonymous unions and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int c : 10;
        };
        long d;
        struct {
            short e : 7;
            short f : 9;
        };
    };
    
    /* Function pointer array returning pointer to array */
    int (*(*func_ptr_arr[2])(void))[3];
    
    /* Nested array in union */
    union {
        int matrix[2][3][4];
        double tensor[2][2][2][2];
    } data;
};

/* Macro to generate complex types with varying sizes */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]); \
    typedef struct { \
        int (*callbacks[n])(int, ...); \
        char buffer[n][n][n]; \
    } Container##n;

/* GCC attributes with nested parentheses */
typedef int __attribute__((aligned(32))) AlignedInt;
typedef void (*PrintFunc)(const char*, ...) 
    __attribute__((format(printf, 1, 2)));

/* Complex function prototype with all delimiter types */
extern void process_data(
    int (*(*callback)(int (*arr[][10])(float)))[20],
    struct Outer (*containers)[5],
    ...) __attribute__((sentinel));

#endif /* COMPLEX_TYPES_H */
