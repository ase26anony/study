#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer with nested parentheses */
typedef int (*(*complex_callback)(int (*)(float)))[10];

/* Multi-dimensional array type with function pointer elements */
typedef void (*(*array_of_func_ptrs[5][3])(char, ...))(double);

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
        unsigned long long f;
    };
    
    /* Array of function pointers returning pointers to arrays */
    int (*(*func_ptr_arr[2])(void))[3];
    
    /* Nested array with computed size */
    char data[sizeof(struct {
        int x;
        double y;
        char z[20];
    })];
};

/* GCC attributes with nested parentheses */
#define FORMAT_ATTR __attribute__((format(printf, 2, 3)))
#define PACKED_ATTR __attribute__((packed, aligned(32)))

/* Macro to generate complex types with varying delimiter nesting */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]); \
    typedef struct { \
        int matrix[n][n*2]; \
        void (*handlers[n])(int, ...); \
    } ComplexStruct##n

/* Function declarations with complex parameter types */
void process_matrix(int matrix[][10], int rows) FORMAT_ATTR;
complex_callback* create_callbacks(int count, 
    int (*(*factory)(int, char(*)[10]))(void));

#endif /* COMPLEX_TYPES_H */
