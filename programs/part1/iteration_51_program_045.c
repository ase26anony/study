#ifndef TEST_NESTED_H
#define TEST_NESTED_H

/* Complex nested array with parenthesized size expressions */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Struct with nested array and bit-fields */
typedef struct GTY(()) nested_struct {
    int x;
    char arr[5];
    unsigned int flags:4;
    unsigned int mode:3;
} nested_struct_t;

/* Union containing struct with array */
typedef union {
    struct {
        int x;
        char arr[5];
    };
    long l;
    double d[(2+3)];
} nested_union_t;

#endif /* TEST_NESTED_H */
