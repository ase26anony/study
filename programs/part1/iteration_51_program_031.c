#ifndef TEST_NESTED_H
#define TEST_NESTED_H

/* Complex nested array with parenthesized size expressions */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Nested struct with arrays and bit-fields */
typedef struct {
    int flags : 3;
    char name[(10 + 2)];
    struct {
        int x;
        int y;
        int z[3];
    } coord;
} nested_struct_t;

/* Function pointer with nested parameter list containing arrays */
typedef void (*signal_handler_t)(int sig, void (*cleanup)(char *msg[10]));

#endif /* TEST_NESTED_H */
