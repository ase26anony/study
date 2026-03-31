#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];
typedef void (*(*signal_handler)(int, void (*)(int)))(void);

/* Nested array types with parentheses for grouping */
typedef int (*array_of_5_funcs[5])(char, ...);
typedef float (*(*matrix_processor)[3][4])(double, ...);

/* GCC attributes with nested parentheses */
#define FORMAT_ATTR __attribute__((format(printf, 2, 3)))
#define ALIGNED_ATTR __attribute__((aligned(32)))
#define PACKED_ATTR __attribute__((packed))

/* Macro to generate complex types with varying delimiter nesting */
#define MAKE_COMPLEX_TYPE(n) int (*(*var##n)[n])(char (*)[n])
#define MAKE_NESTED_ARRAY(n) int (*(*nested_arr##n)[n][n+1])(void)

/* Structure with deeply nested delimiters */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int c : 10;
        } bits;
        long long_value;
        double double_value;
    } data;
    
    /* Array of function pointers returning pointers to arrays */
    int (*(*func_ptr_arr[2])(void))[3];
    
    /* Nested anonymous struct with bit-fields */
    struct {
        unsigned x : 4;
        unsigned y : 4;
        struct {
            unsigned p : 2;
            unsigned q : 6;
        } nested_bits;
    };
    
    /* Function pointer with complex signature */
    void (*(*complex_handler)(int, 
        struct Outer* (*)(int, ...), 
        void (*callback)(int, int)))(char*);
} ALIGNED_ATTR;

/* Union with variadic function pointer */
union VariadicUnion {
    int (*(*var_func)(int, ...))[5];
    struct {
        int (*simple_func)(void);
        int (*(*complex_arr[3])(int))[2];
    } func_group;
};

#endif /* COMPLEX_TYPES_H */
