#ifndef TEST_NESTED_H
#define TEST_NESTED_H

/* Complex nested array with parenthesized size expressions */
typedef int multi_dim_array_t[5][(sizeof(long) > 4) ? 10 : 20][(2 + 3)];

/* Struct with nested array and bit-fields */
typedef struct GTY(()) nested_container {
    int flags:8;
    char data[((16) + (8))];
    struct GTY((skip)) nested_container *next;
    union {
        int i;
        float f;
        char arr[3][2];
    } GTY((tag("union_type"))) value;
} nested_container_t;

/* Function pointer with deeply nested parameter list */
typedef void (*complex_callback_t)(
    int (*inner_func)(char param[10], 
                      struct {int x; int y;} point),
    void (*another)(int, ...)
);

#endif /* TEST_NESTED_H */
