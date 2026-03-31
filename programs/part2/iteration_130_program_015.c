#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex type definitions with nested delimiters */

/* Function pointer with nested parameter list */
typedef int (*(*complex_callback)(int (*)(float), 
                                  void (*)(int, ...)))[10];

/* Multi-dimensional array type */
typedef int (*(*array_of_func_ptrs[5][3])(char, ...))[7];

/* Structure with deeply nested anonymous structs and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int c : (sizeof(int) * 8 - 8);
        };
        struct {
            long d : 12;
            long e : 20;
        };
        long f;
    };
    
    /* Array of function pointers returning pointers to arrays */
    int (*(*func_ptr_arr[2])(void))[3];
    
    /* Nested structure with array of pointers to functions */
    struct Inner {
        char (*(*string_proc)(int, char (*)[10]))[20];
        double matrix[4][4];
    } inner;
    
    /* Pointer to function with complex signature */
    void (*(*signal_handler)(int, 
                             void (*)(int), 
                             struct Outer*))(
        int, 
        __attribute__((aligned(32))) void*
    );
};

/* Union with nested structures */
union MegaUnion {
    struct {
        int (*(*nested_fp)(int (*)[5]))[10];
        char buffer[100];
    } s1;
    
    struct {
        float (*(*float_matrix)[3][4])(double, ...);
        long double ld;
    } s2;
    
    /* Anonymous struct with bit-fields */
    struct {
        unsigned int flags : 8;
        unsigned int mode : 4;
        unsigned int : (32 - 12); /* Padding */
    };
};

/* Macro to generate complex types with varying sizes */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n], \
                        __attribute__((format(printf, 2, 3))) void (*)(const char*, ...))

/* Macro for nested array types */
#define NESTED_ARRAY_TYPE(depth) \
    int (*array##depth)[depth][depth+1][depth+2]

#endif /* COMPLEX_TYPES_H */
