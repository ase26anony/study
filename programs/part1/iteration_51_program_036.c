#ifndef TEST_NESTED_H
#define TEST_NESTED_H

/* Complex nested parentheses in function pointer types */
typedef int (*complex_func_ptr)(int (*inner)(char[10]), 
                                 struct {int a; int b;});

/* Multi-dimensional array with parenthesized size expressions */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];
typedef char three_d_array[(2+3)][(sizeof(int)*2)][10];

/* Nested struct/union definitions within typedef */
typedef union {
    struct {
        int x;
        char arr[5];
        struct {
            short s;
            long l;
        } inner;
    };
    long l;
    double d[(2+3)];
} nested_union_t;

/* Function pointer returning function pointer */
typedef void (*(*func_factory)(int))(char);

/* Array of function pointers */
typedef int (*fp_array_t[5])(void);

#endif /* TEST_NESTED_H */
