#ifndef TEST_COMPLEX_TYPES_H
#define TEST_COMPLEX_TYPES_H

/* Complex type definitions with nested delimiters */

/* 1. Function pointers with deeply nested signatures */
typedef int (*(*complex_callback)(int (*)(float, double), 
                                  char *(*)[10]))[5];

/* 2. Multi-dimensional function pointer array */
typedef void (*(*func_ptr_matrix[3][4])(int, ...))(char);

/* 3. Nested structure with anonymous unions/structs */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int c : (sizeof(int) * 8 - 8);
        };
        struct {
            long d;
            short e;
        };
    };
    
    /* Array of function pointers returning pointers to arrays */
    int (*(*func_ptr_arr[2])(void))[3];
    
    /* Nested structure with bit-fields */
    struct Inner {
        int x : 10;
        int y : (20 - sizeof(char));
        int z[2][3];
    } inner;
};

/* 4. Type with GCC attributes containing nested parentheses */
typedef int __attribute__((aligned(32))) 
        (*(*aligned_func_ptr)(
            char __attribute__((aligned(16))),
            void *__attribute__((nonnull(1, 2)))
        ))[10];

/* 5. Variadic function pointer type */
typedef int (*printf_like_func)(const char *__attribute__((format_arg(1))), 
                                ...) 
        __attribute__((format(printf, 1, 2)));

/* 6. Complex array type with nested dimensions */
typedef int (*(*nested_array_ptr)[5][10])[3][7];

/* Macro to generate complex types with varying nested delimiters */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n], \
                        void (*)(int [n][n*2], \
                                 struct { int a[n]; char b; }))

/* Function declarations using complex types */
complex_callback get_callback(void);
void process_matrix(func_ptr_matrix *matrix);
int analyze_structure(struct Outer *outer);

#endif /* TEST_COMPLEX_TYPES_H */
