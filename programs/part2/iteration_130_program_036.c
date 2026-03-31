#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex type definitions with nested delimiters */
typedef int (*array_of_5_funcs[5])(char, ...);

/* Function pointer with complex signature */
typedef int (*(*complex_callback)(int (*)(float)))[10];

/* Nested structure with anonymous structs and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
        };
        long c;
    };
    int (*(*func_ptr_arr[2])(void))[3];
};

/* Macro to generate complex types with nested delimiters */
#define MAKE_COMPLEX_TYPE(n) int (*(*var##n)[n])(char (*)[n])

/* Attribute with nested parentheses */
#define FORMAT_ATTR __attribute__((format(printf, 2, 3)))
#define ALIGNED_ATTR __attribute__((aligned(32)))

#endif /* COMPLEX_TYPES_H */
