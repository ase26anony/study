#ifndef TEST_NESTED_H
#define TEST_NESTED_H

/* Complex nested parentheses in function pointer types */
typedef int (*deep_nested_fp)(int (*level1)(char[10]), 
                               void (*level2)(struct {int x; double y;}), 
                               float (*level3)(int, ...));

/* Multi-dimensional arrays with parenthesized expressions */
typedef int complex_matrix[5][(sizeof(long) > 4) ? 10 : 20][(2+3)*4];

/* Nested struct/union with arrays and bit-fields */
typedef union {
    struct {
        int flags : 4;
        char data[8][(16/2)];
        struct {
            long counter;
            short offsets[3];
        } inner;
    } s;
    double alignment[((8+7)/8)];
} nested_container_t;

/* Function pointer returning array pointer */
typedef char (*(*signal_handler)(int signo, void *ctx))[256];

#endif /* TEST_NESTED_H */
