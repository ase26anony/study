#ifndef TEST_NESTED_H
#define TEST_NESTED_H

/* Complex nested array with parenthesized size expressions */
typedef int multi_dim_array_t[5][(sizeof(long) > 4) ? 10 : 20][(1 << 3)];

/* Struct with nested array and bit-fields */
typedef struct GTY(()) deeply_nested {
    int flags:((sizeof(int)*8) - 2);
    char data[((10) + (5))];
    struct deeply_nested * GTY((skip)) next;
    void (* GTY((tag("callback"))) callback)(int, char[10]);
} deeply_nested_t;

/* Union containing anonymous struct with array */
typedef union GTY((desc("%1.type"))) {
    struct {
        int type;
        char buffer[(256)];
    };
    long long_data;
    double double_data[(4)];
} variant_data_t;

#endif /* TEST_NESTED_H */
