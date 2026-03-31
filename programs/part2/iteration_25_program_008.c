/* Test file 1: Deeply nested structs and unions */
#ifndef DEEP_NESTED_STRUCTS_H
#define DEEP_NESTED_STRUCTS_H

/* Level 1: Basic nested struct */
struct Level1 {
    int a;
    struct {
        char b;
        union {
            int c;
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
        char *names[10];
        struct {
            int (*callback)(void);
            union {
                void (*func1)(int, char);
                int (*func2)(struct ComplexArray *);
            } func_union;
        } operations;
    } handler;
};

/* Level 3: Deep nesting with anonymous structs/unions */
struct DeepNest {
    struct {
        union {
            struct {
                int a;
                struct {
                    char b;
                    union {
                        int c;
                        struct {
                            double d;
                            float e[2][3];
                        } deep;
                    } u;
                } inner;
            } s;
            long l;
        } u1;
    } level1;
    
    struct {
        int x;
        struct {
            int y;
            struct {
                int z;
                struct {
                    int w;
                } deepest;
            } mid;
        } mid2;
    } level2;
};

/* Level 4: Bit-fields and mixed declarations */
struct MixedFields {
    unsigned int flags : 4;
    signed int value : 12;
    struct {
        unsigned char type : 2;
        unsigned char : 6; /* Padding */
        union {
            struct {
                int a : 8;
                int b : 8;
                int c : 16;
            } parts;
            unsigned int whole;
        } data;
    } control;
    
    int array[2][3];
    struct {
        char *name;
        int id;
    } metadata;
};

/* Designated initializers style in type context */
struct WithDesignators {
    struct {
        int x;
        int y;
    } point;
    
    int values[5];
    
    union {
        struct {
            int a;
            int b;
        } s;
        long l;
    } u;
};

#endif /* DEEP_NESTED_STRUCTS_H */
