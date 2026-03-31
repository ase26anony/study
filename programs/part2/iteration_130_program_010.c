/* complex_types.h */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Nested attribute with parentheses */
#define FORMAT_ATTR __attribute__((format(printf, 2, 3)))
#define ALIGNED_ATTR(n) __attribute__((aligned(n)))

/* Macro generating complex types with nested delimiters */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]) ALIGNED_ATTR(32)

/* Complex function pointer type with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];

/* Multi-dimensional array type */
typedef int matrix_t[3][4];

/* Structure with nested anonymous structs and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
        };
        long c;
    };
    int (*(*func_ptr_arr[2])(void))[3];
    MAKE_COMPLEX_TYPE(5) *ptr;
};

/* Type definition with variadic function pointers */
typedef int (*array_of_5_funcs[5])(char, ...) FORMAT_ATTR;

/* Function declarations using complex types */
void process_callback(complex_callback cb);
int (*(*get_func_array(void))[3])(void);

#endif /* COMPLEX_TYPES_H */
