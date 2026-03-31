/* Test file for gengtype parser - deeply nested structures */

#ifndef DEEP_NESTED_STRUCTS_H
#define DEEP_NESTED_STRUCTS_H

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
struct ComplexArrayStruct {
    int matrix[3][4][5];
    struct {
        char *names[10];
        struct {
            int (*callback)(void);
            void (*handlers[5])(int, char);
        } funcs;
    } data;
};

/* Level 3: Deeply nested anonymous structs/unions */
struct DeepNest {
    struct {
        union {
            struct {
                int a;
                struct {
                    char b;
                    union {
                        short c;
                        struct {
                            float d[2][3];
                        } arr_struct;
                    } deep_u;
                } level4;
            } level3;
            long alternate;
        } level2;
    } level1;
    int final;
};

/* Level 4: Bit-fields with nested structs */
struct BitFieldStruct {
    unsigned int flags : 4;
    struct {
        unsigned int a : 2;
        unsigned int b : 3;
        struct {
            unsigned int c : 1;
            unsigned int d : 5;
        } nested_bits;
    } bit_groups;
    int regular_field;
};

/* Level 5: Designated initializers in type definitions */
struct WithInitializer {
    int x;
    struct {
        int a;
        int b;
        int c[3];
    } inner;
    union {
        char ch;
        int num;
        struct {
            float f;
            double d;
        } fp;
    } value;
};

#endif /* DEEP_NESTED_STRUCTS_H */
