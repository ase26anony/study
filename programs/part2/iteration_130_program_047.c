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
    };
    int (*(*func_ptr_arr[2])(void))[3];
    struct {
        int (*nested_func)(struct Outer *);
        union {
            char *str;
            void *ptr;
        } u;
    } inner;
};

/* GCC attributes with nested parentheses */
#define ALIGNED_TYPE __attribute__((aligned(32)))
#define FORMAT_PRINTF __attribute__((format(printf, 2, 3)))

/* Macro to generate complex types with varying sizes */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]) ALIGNED_TYPE

/* Complex function declaration with attributes */
FORMAT_PRINTF
int complex_printf(void (*callback)(int), const char *fmt, ...);

#endif /* COMPLEX_TYPES_H */
