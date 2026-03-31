/* Test file for deeply nested struct/union constructs */
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

/* Level 2: Anonymous structs/unions */
struct OuterContainer {
    struct {
        union {
            int x;
            struct {
                char y;
                struct {
                    unsigned z: 4;
                    unsigned w: 12;
                } bits;
            } s;
        } anon_union;
    } anon_struct;
    
    /* Array within nested struct */
    struct Matrix {
        int data[3][3];
        struct {
            float values[2][2][2];
        } cube;
    } mat;
};

/* Level 3: Mixed nesting with bitfields */
struct DeepNest {
    struct A {
        union B {
            struct C {
                int field1;
                struct D {
                    char field2;
                    union E {
                        short f1;
                        long f2;
                        struct F {
                            unsigned b1: 1;
                            unsigned b2: 3;
                            unsigned b3: 4;
                        } bits;
                    } ue;
                } d;
            } c;
            int alt;
        } ub;
    } a;
    
    /* Multi-dimensional array */
    int deep_array[2][3][4][5];
};

/* Level 4: Designated initializers (in type context) */
struct WithDesignators {
    struct {
        int x;
        int y;
    } point;
    
    union {
        struct {
            int a;
            int b;
        } s;
        int arr[2];
    } data;
};

#endif /* NESTED_STRUCTS_H */
