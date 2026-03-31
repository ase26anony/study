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
struct MatrixContainer {
    struct {
        int rows;
        int cols;
        int data[10][20];
    } matrix;
    union {
        struct {
            float (*transform)[4][4];
        } trans;
        void (*callback)(void);
    } ops;
};

/* Level 3: Deeply nested anonymous structs/unions */
struct DeepNest {
    struct {
        union {
            struct {
                int a;
                struct {
                    char b[5];
                    struct {
                        short c;
                        union {
                            long d;
                            float e;
                        } deepest;
                    } inner;
                } mid;
            } level1;
            void *ptr;
        } u1;
    } s1;
    int arr[3][4][5];
};

/* Level 4: Bit-fields and mixed nesting */
struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    struct {
        unsigned int c : 2;
        unsigned int d : 6;
        union {
            struct {
                unsigned int e : 1;
                unsigned int f : 7;
            } bits;
            char byte;
        } u;
    } inner;
    int regular_array[2][3];
};

/* Designated initializers in type context (GCC extension) */
struct WithDesignators {
    struct {
        int x;
        int y;
    } point;
    int values[5];
} global_var = { .point = { .x = 1, .y = 2 }, .values = { [0] = 10, [4] = 20 } };

#endif /* NESTED_STRUCTS_H */
