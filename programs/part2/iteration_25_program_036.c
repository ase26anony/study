/* Test file 1: Deeply nested structs and unions */

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
    } inner;
};

/* Level 2: Array within nested struct */
struct MatrixContainer {
    struct {
        int rows;
        int cols;
        int data[10][20];  /* Nested array brackets */
    } matrix;
    union {
        struct {
            char (*callback)(int, float);  /* Function pointer */
        } s;
        void *ptr;
    } config;
};

/* Level 3: Deep nesting with anonymous structs/unions */
struct DeepNest {
    struct {
        union {
            struct {
                int a;
                struct {
                    char b[5][10];  /* Multi-dimensional array */
                    union {
                        short c;
                        long d[3];
                    } u;
                } s;
            } inner;
            long long big;
        } u1;
    } level1;
    struct {
        int x;
        struct {
            int y;
            struct {
                int z;
                int arr[2][3][4];  /* Triple nested array */
            } deep;
        } mid;
    } level2;
};

/* Level 4: Mixed nesting with bit-fields */
struct MixedNesting {
    unsigned int flags : 4;
    struct {
        int : 16;  /* Unnamed bit-field */
        signed int value : 12;
        union {
            struct {
                char a : 3;
                char b : 5;
            } bits;
            unsigned char byte;
        } packed;
    } control;
    int matrix[3][3];  /* 2D array */
};

/* Level 5: Struct with designated initializers (in type definition) */
struct WithDesignatedInit {
    int a;
    struct {
        int x;
        int y;
    } point;
    union {
        struct {
            char name[20];
            int id;
        } s;
        void *data;
    } info;
    int arr[5];
};

#endif /* DEEP_NESTED_STRUCTS_H */
