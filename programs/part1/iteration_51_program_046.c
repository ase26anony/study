#ifndef TEST_NESTED_H
#define TEST_NESTED_H

/* Complex function pointer with nested parameter lists */
typedef int (*complex_fp1)(int (*inner)(char[10]), 
                          struct {int a; int b;});

/* Multi-dimensional array with parenthesized size expressions */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Nested union with struct containing arrays */
typedef union {
    struct {
        int x;
        char arr[5];
    };
    long l;
} nested_union_t;

/* Macro expanding to balanced token groups */
#define PTR_TO(T) T*
#define NESTED_ARRAY int[5][5]
#define ARRAY_SIZE_EXPR (sizeof(int*) * 2)

#endif /* TEST_NESTED_H */
