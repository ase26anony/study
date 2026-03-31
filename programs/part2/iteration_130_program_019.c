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
        struct {
            long d : 15;
            long e : 17;
        };
        long f;
    };
    
    /* Array of function pointers returning pointers to arrays */
    int (*(*func_ptr_arr[2])(void))[3];
    
    /* Nested array with computed size */
    double matrix[][4];
};

/* GCC attributes with nested parentheses */
#define ALIGNED_TYPE __attribute__((aligned(32)))
#define FORMAT_PRINTF __attribute__((format(printf, 2, 3)))

/* Macro to generate complex types with varying delimiter nesting */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]); \
    typedef struct { \
        int (*(*member##n)[n])(int (*)(char[n])); \
        union { \
            struct { \
                int bitfield##n : n; \
                int : 32 - n; \
            }; \
            long long_data##n; \
        }; \
    } ComplexStruct##n

/* Generate several complex types */
MAKE_COMPLEX_TYPE(2);
MAKE_COMPLEX_TYPE(4);
MAKE_COMPLEX_TYPE(8);

/* Function declarations with complex parameter types */
FORMAT_PRINTF int debug_print(const char *format, ...);
ALIGNED_TYPE void* aligned_allocator(size_t size);

/* Complex const volatile qualified function pointer */
typedef int (*(* const volatile cv_fp)(const int *restrict))[10];

#endif /* COMPLEX_TYPES_H */
