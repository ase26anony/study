#ifndef TEST_NESTED_H
#define TEST_NESTED_H

/* Complex nested array with parenthesized size expression */
typedef int multi_dim_array_t[5][(sizeof(long) > 4) ? 10 : 20][3];

/* Struct with nested array and bit-fields */
typedef struct GTY(()) deeply_nested_s {
    struct GTY(()) inner {
        int x;
        char arr[5][(2+3)];
        unsigned int bits:4;
        unsigned int more_bits:((sizeof(int)*8)-4);
    } inner_struct;
    
    union {
        long l;
        double d;
        struct {
            short s;
            char c[((10/2)+1)];
        } nested;
    } data_union;
} deeply_nested_t;

/* Function pointer type with multiple levels of nesting */
typedef void (*complex_callback_t)(
    int (*inner_func)(char param[10], 
                      struct {int a; double b;} anonymous),
    void (*another)(int, ...)
);

#endif /* TEST_NESTED_H */
