/* Test file 1: Deeply nested structs and unions */
#ifndef NESTED_STRUCTS_H
#define NESTED_STRUCTS_H

/* Level 1: Basic nested struct */
struct Level1 {
    int a;
    struct {
        char b;
        union {
            short c;
            long d;
            struct {
                float e;
                double f;
            } inner;
        } u;
    } nested;
};

/* Level 2: Array within nested struct */
struct ComplexArray {
    int matrix[3][4][5];
    struct {
        char *ptr_array[10];
        union {
            int (*func_ptr)(void);
            void (*void_ptr)(int, char);
        } callbacks;
    } data;
};

/* Level 3: Deep nesting with bitfields */
struct DeepNested {
    unsigned int flags : 4;
    struct {
        struct {
            union {
                struct {
                    int : 16;
                    int field1 : 8;
                    int field2 : 8;
                } bits;
                unsigned int raw;
            } u;
        } inner;
        int arr[2][3];
    } container;
};

/* Level 4: Mixed delimiters in initializers */
struct WithInitializer {
    int values[4];
    struct {
        char chars[3];
        float floats[2];
    } pairs;
} global_var = {
    .values = {1, 2, {3, 4}},
    .pairs = {
        .chars = {'a', 'b', 'c'},
        .floats = {1.0f, 2.0f}
    }
};

#endif /* NESTED_STRUCTS_H */
