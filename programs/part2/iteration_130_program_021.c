#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];
typedef void (*(**signal_handler)(int, siginfo_t*, void*))(int);

/* Multi-dimensional array type with function pointers */
typedef int (*(*array_of_func_ptrs[5])(void))[3];

/* Nested structure with anonymous structs and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int c : 10;
        };
        struct {
            long d : 12;
            long e : 20;
        };
        unsigned long long f;
    };
    
    /* Function pointer array member */
    int (*(*func_ptr_arr[2])(void))[3];
    
    /* Nested structure with array */
    struct Inner {
        int matrix[2][3][4];
        char (*(*string_array[5]))[20];
    } inner;
};

/* Complex typedef with variadic functions */
typedef int (*array_of_variadic_funcs[5])(char, ...);

/* Macro to generate complex types with nested delimiters */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*complex_array_##n)[n])(char (*)[n]); \
    struct NestedStruct_##n { \
        int arr[n][n+1][n+2]; \
        void (*funcs[n])(int (*)[n], ...); \
    };

/* GCC attributes with nested parentheses */
#define FORMAT_ATTR __attribute__((format(printf, 2, 3)))
#define ALIGNED_ATTR __attribute__((aligned(32)))
#define PACKED_ATTR __attribute__((packed))

/* Structure with attributes containing nested parentheses */
struct AttributedStruct {
    int data;
    void (*print_func)(const char *, ...) FORMAT_ATTR;
} ALIGNED_ATTR PACKED_ATTR;

#endif /* COMPLEX_TYPES_H */
