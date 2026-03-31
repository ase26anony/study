#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];
typedef void (*(*(*nested_func_ptr)(char (*)[5]))(int, ...))(double);

/* Structure with deeply nested anonymous structs and bit-fields */
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
        unsigned long long f;
    };
    
    /* Array of function pointers returning pointers to arrays */
    int (*(*func_ptr_arr[3])(void))[4];
    
    /* Nested structure with bit-fields */
    struct Inner {
        struct {
            int x : 8;
            int y : 8;
            int z : 16;
        } nested_bits;
        
        /* Function pointer with complex signature */
        char (*(*(*inner_func)(int (*)(char **)))[2])(void);
    } inner;
};

/* Union with anonymous struct and array initializer */
union ComplexUnion {
    struct {
        int matrix[2][3];
        float (*(*fp)(int[2][2]))[3];
    };
    long double ld_array[4];
};

/* Macro to generate complex types with varying delimiter nesting */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]); \
    typedef struct { \
        int (*(*arr##n[n])(int (*)(float[n])))[n]; \
        union { \
            int a : n; \
            long b : (n * 2); \
        } bits; \
    } Type##n##_t

/* Generate several complex types */
MAKE_COMPLEX_TYPE(2);
MAKE_COMPLEX_TYPE(4);
MAKE_COMPLEX_TYPE(8);

/* GCC attributes with nested parentheses */
#ifdef __GNUC__
#define FORMAT_ATTR __attribute__((format(printf, 2, 3)))
#define ALIGNED_ATTR __attribute__((aligned(32)))
#define PACKED_ATTR __attribute__((packed))
#define SECTION_ATTR __attribute__((section(".special_section")))
#else
#define FORMAT_ATTR
#define ALIGNED_ATTR
#define PACKED_ATTR
#define SECTION_ATTR
#endif

/* Structure with various GCC attributes */
struct AttributedStruct {
    int data ALIGNED_ATTR;
    char buffer[64] PACKED_ATTR;
    void (*printf_func)(const char *, ...) FORMAT_ATTR;
} SECTION_ATTR;

/* Complex array type with initializer in declaration */
extern int multi_dim_array[3][4][2];

/* Function declarations with complex parameter types */
void process_complex(complex_callback cb, 
                     int (*(*param)[5])(char (*)[3]),
                     struct Outer *outer);

int (*(*get_func_array(void))[3])(int, float);

#endif /* COMPLEX_TYPES_H */
