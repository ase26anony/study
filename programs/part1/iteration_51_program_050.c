#ifndef TEST_NESTED_H
#define TEST_NESTED_H

/* Complex nested function pointer type with multiple balanced groups */
typedef int (*deep_nested_fp)(
    int (*callback)(char[10], struct {int x; int y;}),
    void (*cleanup)(int matrix[5][(sizeof(long) > 4) ? 10 : 20]),
    union {
        struct { int a; char b[7]; };
        long long l;
    } data
);

/* Multi-dimensional array with parenthesized expressions */
typedef int complex_matrix_t[
    5][
    (sizeof(void*) == 8) ? 16 : 8
][
    (1 << 3) + 2
];

/* Struct with nested arrays and bit-fields */
typedef struct GTY(()) nested_container {
    struct GTY((tag("1"))) inner {
        unsigned int flags:4;
        char name[(10 + 5) * 2];
        int * GTY((skip)) pointers[3];
    } sections[2];
    
    union {
        struct {
            int x;
            int y[((5) + (3))];
        };
        double dbl;
    } coord;
} nested_container_t;

#endif /* TEST_NESTED_H */
