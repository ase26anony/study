/* { dg-do compile } */
/* Complex type definitions to exercise gengtype's balanced delimiter parsing */

#ifndef FILE1_H
#define FILE1_H

/* 1. Function pointers with nested argument lists */
typedef int (*callback_t)(int (*)(char), double);
typedef void (*(*signal_handler_t)(int sig, void (*func)(int)))(int);

/* 2. Multi-dimensional arrays with nested size expressions */
typedef int matrix_t[10][(sizeof(int) > 2) ? 5 : 3];
typedef struct {
    int len;
    int arr[];
} flexible_array_t;

/* 3. Deeply nested function pointer returning pointer to array */
typedef int (*(*complex_callback_t)(void))[10];

/* 4. Struct containing array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*(*nested_ops[3])(int))[2];
};

/* 5. Union with nested initializer-style type */
union NestedUnion {
    struct {
        int (*func_ptr)(int, int);
        char arr[((sizeof(int) + 3) & ~3)];
    } inner;
    long long data;
};

/* 6. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]
DECLARE_COMPLEX(5);
DECLARE_COMPLEX(10);

/* 7. Type with all three delimiters deeply nested */
typedef struct {
    int (*get_value)(int index, int (*validator)(char));
    int data[3][(4 + 1)];
    union {
        struct { int x; int y; } point;
        int (*transform)(int (*)(int), int);
    } u;
} uber_complex_t;

/* 8. Variable-length array in struct */
struct VLAContainer {
    int count;
    int items[/* flexible */];
};

/* 9. Function pointer with array parameter */
typedef void (*array_processor_t)(int arr[((16/sizeof(int)) + 1)], int size);

/* 10. Nested parentheses in cast-like context */
typedef int *(*(*nested_ptr_fun)(int (*)(double)))(char);

#endif /* FILE1_H */
