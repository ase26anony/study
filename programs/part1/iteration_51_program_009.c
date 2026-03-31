#ifndef TEST_NESTED_H
#define TEST_NESTED_H

/* Complex nested array with parenthesized size expressions */
#define ARRAY_SIZE (sizeof(long) > 4 ? 16 : 8)
typedef int nested_array_t[5][(ARRAY_SIZE)][(10 + 2)];

/* Struct with nested arrays and bit-fields */
typedef struct GTY(()) deeply_nested {
    int flags:8;
    char data[4][(sizeof(int) * 2)];
    struct GTY((skip)) deeply_nested *next;
    union {
        long l;
        double d[2][3];
    } GTY((tag("union_data"))) u;
} deeply_nested_t;

/* Macro expanding to balanced token groups */
#define FUNCTION_PTR(RET, ARGS) RET (*)(ARGS)
#define COMPLEX_ARGS int, char[(ARRAY_SIZE)], void (*)(int)

#endif /* TEST_NESTED_H */
