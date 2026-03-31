/* Test file 1: Deeply nested struct/union definitions */

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
    } inner;
};

/* Level 2: Anonymous structs/unions */
struct OuterContainer {
    struct {
        int x;
        union {
            char y;
            struct {
                int z[3];
                struct {
                    short a;
                    long b;
                } nested;
            } s;
        } anon_union;
    };
    
    struct Named {
        int (*callback)(struct OuterContainer *self);
        union {
            void *ptr;
            int (*func_ptrs[5])(int, char);
        } complex_union;
    } named_member;
};

/* Level 3: Arrays within nested structs */
struct MatrixHolder {
    int matrix[3][4][5];
    struct {
        char strings[10][20];
        struct {
            float coords[2][3];
            union {
                int ints[4];
                double doubles[2][2];
            } data;
        } spatial;
    } container;
};

/* Level 4: Bit-fields and designated initializers style */
struct BitFieldStruct {
    unsigned int a : 3;
    unsigned int b : 5;
    struct {
        unsigned int c : 2;
        unsigned int d : 4;
        union {
            unsigned int e : 1;
            unsigned int f : 7;
        } flags;
    } bits;
};

/* Level 5: Mixed nesting with all delimiters */
struct UltimateNest {
    int (*func_array[3])(char, float);  /* Mix of [] and () */
    struct {
        union {
            void *(*getter)(int index);
            int (*setter)(void *ptr, int value);
        } ops;
        int matrix[2][2];
    } operations;
    
    struct {
        struct {
            char (*string_processor)(const char *input, int len);
        } text;
    } processors;
};

#endif /* NESTED_STRUCTS_H */
