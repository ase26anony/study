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
            } inner;
        } u;
    } nested;
};

/* Level 2: Array within nested struct */
struct MatrixContainer {
    struct {
        int rows;
        int cols;
        int data[10][20];  /* Nested brackets */
    } matrix;
    union {
        struct {
            char *name;
            int id;
        } s;
        void *ptr;
    } metadata;
};

/* Level 3: Deep nesting with anonymous structs/unions */
struct DeepNest {
    struct {
        union {
            struct {
                int x;
                struct {
                    char y;
                    union {
                        short z;
                        long w;
                        struct {
                            float a;
                            double b;
                            int c[5][5][5];  /* Triple nested array */
                        } deepest;
                    } u;
                } inner;
            } s;
            void *alternate;
        } choice;
    } container;
};

/* Level 4: Bit-fields and mixed nesting */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    struct {
        signed int value : 10;
        union {
            struct {
                unsigned char a : 2;
                unsigned char b : 2;
                unsigned char c : 2;
                unsigned char d : 2;
            } bits;
            unsigned char byte;
        } packed;
    } data;
    int array[3][4];  /* 2D array */
};

/* Level 5: Designated initializers (in type context) */
struct DesignatedInit {
    struct {
        int x;
        int y;
        int z;
    } point;
    union {
        struct {
            int r;
            int g;
            int b;
            int a;
        } rgba;
        unsigned int color;
    } value;
    int matrix[2][2];
};

#endif /* DEEP_NESTED_STRUCTS_H */
