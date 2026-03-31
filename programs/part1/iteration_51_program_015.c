#ifndef TEST_NESTED_H
#define TEST_NESTED_H

/* Complex nested function pointer with multiple balanced groups */
typedef int (*nested_fp_t)(int (*inner)(char[10]), 
                           struct {int a; int b;},
                           void (*callback)(int, int));

/* Multi-dimensional array with parenthesized size expressions */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Struct with nested array and bit-field */
typedef struct GTY((user)) {
    int x;
    char arr[5];
    unsigned int flags:4;
    unsigned int status:2;
} nested_struct_t;

/* Union containing struct with array */
typedef union {
    struct {
        int x;
        char arr[5][(2+3)];
    };
    long l;
    double d[3];
} nested_union_t;

#endif /* TEST_NESTED_H */
