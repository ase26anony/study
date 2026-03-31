/* Test file 1: Deeply nested structs and unions */
#ifndef DEEPLY_NESTED_STRUCTS_H
#define DEEPLY_NESTED_STRUCTS_H

/* Level 1: Basic nested struct */
struct Level1 {
    int a;
    struct {
        char b;
        union {
            int c;
            struct {
                float d;
                double e;
            } inner;
        } u;
    } nested;
};

/* Level 2: Array within nested struct */
struct ComplexArray {
    int matrix[3][4][5];
    struct {
        char *strings[10];
        union {
            int (*func_ptrs[5])(void);
            struct {
                void (*callback)(int, char);
            } cb_struct;
        } u_array;
    } container;
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
                        float c;
                        struct {
                            double d;
                            int e[2][3];
                        } deepest;
                    } u_inner;
                } mid;
            } inner;
            long long alternate;
        } choice;
    } wrapper;
    int final;
};

/* Level 4: Bit-fields with nested structs */
struct BitFieldStruct {
    unsigned int flags : 4;
    struct {
        unsigned int a : 2;
        unsigned int b : 3;
        union {
            unsigned int c : 1;
            unsigned int d : 5;
        } bits;
    } packed;
    int normal_field;
};

/* Level 5: Designated initializers (in type context) */
struct WithDesignators {
    struct {
        int x;
        int y;
    } point;
    int values[4];
    union {
        char c;
        int i;
        struct {
            float f;
            double d;
        } fs;
    } data;
};

#endif /* DEEPLY_NESTED_STRUCTS_H */
