#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];

/* Multi-dimensional array type with function pointer elements */
typedef void (*(*array_of_funcs[5][3])(char, ...))(double);

/* Nested structure with anonymous members and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int : 24; /* Unnamed bit-field */
        };
        long c;
    };
    
    /* Array of function pointers returning pointers to arrays */
    int (*(*func_ptr_arr[2])(void))[3];
    
    /* Nested array with computed size */
    char data[sizeof(struct { int x; double y; })];
};

/* GCC attributes with nested parentheses */
typedef int __attribute__((aligned(32))) aligned_int;
typedef void (*printf_like_fn)(const char *, ...) 
    __attribute__((format(printf, 1, 2)));

/* Macro generating complex types with nested delimiters */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]) __attribute__((deprecated))

/* Variadic macro with nested parentheses */
#define WITH_ATTRIBUTES(type, ...) \
    type __attribute__((__VA_ARGS__))

#endif /* COMPLEX_TYPES_H */
