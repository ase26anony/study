#ifndef TEST_COMPLEX_TYPES_H
#define TEST_COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];
typedef void (*(*signal_handler)(int, void (*)(int)))(int);

/* Nested array and function pointer combinations */
typedef int (*(*array_of_func_ptrs[5])(void))[3];
typedef char (*(*(*nested_fp)(int))[4])(double);

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
    
    /* Function pointer array member */
    int (*(*func_ptr_arr[2])(void))[3];
    
    /* Nested structure with bit-fields */
    struct Inner {
        struct {
            int x : 4;
            int y : 4;
            int z : 8;
        } nested_bits;
        
        /* Pointer to function returning pointer to array */
        int (*(*inner_func)(int))[5];
    } inner;
};

/* Union with complex type members */
union ComplexUnion {
    int (*(*fp_member)(int (*)[3]))[2];
    struct {
        char (*(*char_fp)(void))[10];
        float matrix[2][3][4];
    } struct_member;
};

/* Variadic function pointer type */
typedef int (*printf_like_func)(const char *, ...);

/* Macro to generate complex types with different nesting levels */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]); \
    typedef struct { \
        int matrix[n][n]; \
        void (*(*callbacks[n])(int, ...))(void); \
    } Type##n;

/* Generate types with different sizes */
MAKE_COMPLEX_TYPE(2)
MAKE_COMPLEX_TYPE(3)
MAKE_COMPLEX_TYPE(5)

/* Attribute specifications with nested parentheses */
#ifdef __GNUC__
#define FORMAT_ATTR __attribute__((format(printf, 2, 3)))
#define ALIGNED_ATTR __attribute__((aligned(32)))
#define PACKED_ATTR __attribute__((packed))
#else
#define FORMAT_ATTR
#define ALIGNED_ATTR
#define PACKED_ATTR
#endif

/* Function declarations with attributes */
void log_message(const char *format, ...) FORMAT_ATTR;
struct PackedStruct {
    char a;
    int b;
    char c;
} PACKED_ATTR;

#endif /* TEST_COMPLEX_TYPES_H */
