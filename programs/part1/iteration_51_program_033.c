/* test_nested.h - Secondary header for nested type definitions */

#ifndef TEST_NESTED_H
#define TEST_NESTED_H

/* Complex function pointer with nested parameter lists */
typedef int (*deep_nested_fp)(
    int (*callback)(char[10], void *),
    struct { 
        int id; 
        char name[(20 + sizeof(int))]; 
    } metadata
);

/* Multi-dimensional array with parenthesized size expressions */
typedef int complex_matrix_t[
    5][
    (sizeof(long long) > 8) ? 10 : 20
][
    (__SIZEOF_POINTER__ * 2)
];

/* Nested union with struct containing arrays */
typedef union {
    struct {
        int counter;
        char buffer[5][(10 - 5)];
        struct {
            short x, y;
        } point;
    } data;
    long long raw[2];
} nested_container_t;

/* Function pointer returning another function pointer */
typedef void (*(*chained_fp)(int))(
    double matrix[3][3],
    void (*cleanup)(void)
);

#endif /* TEST_NESTED_H */
