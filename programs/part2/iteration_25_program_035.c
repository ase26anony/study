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
            } inner_inner;
        } u;
    } nested;
};

/* Level 2: Array within nested struct */
struct ComplexArray {
    int matrix[3][4][5];
    struct {
        char *names[10];
        struct {
            int (*callback)(void);
            union {
                void (*vfunc)(int, char);
                int (*ifunc[2])(float, double);
            } func_union;
        } ops;
    } config;
};

/* Level 3: Deep nesting with bitfields */
struct DeepNested {
    unsigned int flags : 4;
    struct {
        struct {
            union {
                struct {
                    int : 16;
                    unsigned int field1 : 8;
                    signed int field2 : 8;
                } bits;
                long long value;
            } data;
            struct {
                char arr[2][3][4];
            } storage;
        } level_a;
        struct {
            int x;
        } level_b;
    } container;
};

/* Level 4: Anonymous structs/unions */
struct AnonymousMix {
    struct {
        int id;
        union {
            char str[20];
            struct {
                int num;
                float real;
            } pair;
        };
    };
    struct {
        union {
            long counter;
            double precision;
        };
        short tag;
    } meta;
};

/* Level 5: Designated initializers (in type definitions where possible) */
struct WithInitializer {
    struct {
        int a;
        struct {
            char b[3];
            float c;
        } inner;
    } nested = { .a = 42, .inner = { .b = {'x','y','z'}, .c = 3.14 } };
    int arr[2][3] = { [0] = {1, 2, 3}, {4, 5, 6} };
};

#endif /* NESTED_STRUCTS_H */
