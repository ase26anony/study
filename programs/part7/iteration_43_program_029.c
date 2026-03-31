/* { dg-do compile } */
/* Primary header with complex nested delimiter patterns */

#ifndef FILE1_H
#define FILE1_H

/* 1. Function pointers with nested argument lists */
typedef int (*callback_t)(int (*)(char), double);
typedef void (*(*signal_handler_t)(int sig, void (*func)(int)))(int);

/* 2. Multi-dimensional arrays with nested size expressions */
typedef int matrix_t[10][(sizeof(int) > 4) ? 5 : 3];
typedef struct {
    int len;
    int arr[];
} flexible_array_t;

/* 3. Deeply nested function pointer returning pointer to array */
typedef int (*(*complex_callback_t)(void))[10];

/* 4. Struct containing array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*(*nested_ops[3])(double))[2];
};

/* 5. Union with nested initializer-style type */
union NestedUnion {
    struct {
        int (*func_ptr)(int, int);
        char data[((sizeof(int)*8) + 7)/8];
    } inner;
    long long aligner;
};

/* 6. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_TYPE(n) \
    typedef int (*(*fp_type##n)(int))[n]; \
    typedef struct { \
        fp_type##n arr[(n > 0 ? n : 1)]; \
        int (*methods[(n+2)])(int (*)(char), double); \
    } container##n##_t

DECLARE_COMPLEX_TYPE(3);
DECLARE_COMPLEX_TYPE(5);

/* 7. Type with all three delimiters deeply nested */
typedef struct {
    int (*get_value)(int index, int (*validator)(int));
    int data[10][(sizeof(void*) == 8 ? 20 : 10)];
    union {
        struct {
            callback_t cb;
            int matrix[2][{3,4}[0]];  /* Mixed initializer in dimension */
        } s;
        long buffer[((16/sizeof(int)) + 1)];
    } u;
} UltimateType_t;

/* 8. Function prototype with complex return type */
complex_callback_t create_callback(int depth, int (*factory)(int));
void process_matrix(matrix_t m, int (*transform)(int));

#endif /* FILE1_H */
