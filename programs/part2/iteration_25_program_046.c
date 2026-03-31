/* Test file for gengtype parser - deeply nested structs and unions */

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
            int (*callbacks[5])(void);
            union {
                struct {
                    int x;
                    int y;
                } point;
                struct {
                    float radius;
                    float angle;
                } polar;
            } coord;
        } data;
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
                        int c;
                        struct {
                            float d;
                            double e[2][3];
                        } deep;
                    } u2;
                } inner2;
            } s1;
            struct {
                long l;
                struct {
                    short s;
                    struct {
                        char c;
                        int i;
                    } deepest;
                } chain;
            } s2;
        } choice;
    } wrapper;
};

/* Level 4: Mixed delimiters in bit-fields */
struct BitFieldStruct {
    unsigned int a : 3;
    unsigned int b : 5;
    struct {
        unsigned int c : 2;
        unsigned int d : 4;
        union {
            struct {
                unsigned int e : 1;
                unsigned int f : 7;
            } bits1;
            struct {
                unsigned int g : 6;
                unsigned int h : 2;
            } bits2;
        } bit_union;
    } nested_bits;
};

/* Level 5: Designated initializers in type definitions */
struct WithInitializer {
    int array[4][3];
    struct {
        char name[20];
        union {
            int id;
            long serial;
        } ident;
    } info;
};

/* Complex function pointer inside struct */
struct CallbackHolder {
    void (*simple_cb)(int);
    int (*complex_cb)(char *, int (*)(float));
    struct {
        void (*nested_cb)(struct CallbackHolder *);
        int (*array_cb[3])(void);
    } cb_group;
};

#endif /* DEEP_NESTED_STRUCTS_H */
