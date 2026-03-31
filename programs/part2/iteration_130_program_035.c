#ifndef TEST_COMPLEX_TYPES_H
#define TEST_COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];
typedef void (*(**nested_func_ptr_arr[5])(char, ...))(double);

/* Structure with deeply nested anonymous structs and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int : 24; /* Padding with bit-field */
        };
        long c;
        struct {
            short d;
            char e[4];
        } nested;
    };
    int (*(*func_ptr_arr[2])(void))[3];
    char (*(*string_matrix)[4][10])(void);
};

/* Union with complex type members */
union DataContainer {
    struct {
        int (*comparator)(const void *, const void *);
        void (*destructor)(void *);
    } ops;
    struct {
        int (*(*get_matrix)[3][4])(void);
        float (*(*get_vector)[10])(int);
    } math_ops;
};

/* Macro to generate complex array types with nested delimiters */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]); \
    typedef struct { \
        int matrix[n][n]; \
        void (*(*callbacks[n])(int, ...))[n]; \
    } ComplexStruct##n;

/* Apply macro with different sizes */
MAKE_COMPLEX_TYPE(2)
MAKE_COMPLEX_TYPE(5)
MAKE_COMPLEX_TYPE(10)

/* Function declarations with complex parameter types */
extern void process_matrix(int (*(*matrix)[3][4])[5], 
                          void (*(*callback)(int, ...))(double));
extern struct Outer* create_outer(int levels, 
                                 ...) __attribute__((sentinel));

#endif /* TEST_COMPLEX_TYPES_H */
