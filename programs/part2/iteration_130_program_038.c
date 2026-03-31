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

/* GCC attributes with nested parentheses */
#define ALIGNED_STRUCT __attribute__((aligned(32)))
#define FORMAT_PRINTF __attribute__((format(printf, 2, 3)))

/* Macro to generate complex types with nested brackets */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n])

/* Function with complex return type */
complex_callback create_callback(void);

/* Function with GCC attributes */
void debug_print(const char *format, ...) FORMAT_PRINTF;

#endif /* COMPLEX_TYPES_H */
