#ifndef TEST_NESTED_H
#define TEST_NESTED_H

/* Complex function pointer with nested parameter lists */
typedef int (*nested_fp_type)(int (*inner_callback)(char[10]), 
                              struct {int a; int b;} param);

/* Multi-dimensional array with parenthesized size expressions */
typedef int matrix_type[5][(sizeof(long) > 4) ? 10 : 20];

/* Struct with nested array in bit-field context */
typedef struct {
    unsigned flags : 3;
    int data[(10 + 2)];
    struct {
        char name[20];
        int scores[5][5];
    } nested;
} complex_struct_t;

/* Union containing anonymous struct with array */
typedef union {
    struct {
        int x;
        char arr[5];
    };
    long l;
    double matrix[2][(sizeof(int) * 2)];
} nested_union_t;

#endif /* TEST_NESTED_H */
