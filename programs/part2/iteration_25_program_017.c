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
        } u1;
    } nested;
};

/* Level 2: Array within nested struct */
struct Level2 {
    struct {
        int matrix[3][4];
        union {
            char *ptr;
            void (*func)(int, char);
        } choices;
    } container;
    struct {
        struct {
            int deep_field;
        } deeper;
    } wrapper;
};

/* Level 3: Mixed nesting with bitfields */
struct Level3 {
    unsigned int flags : 4;
    struct {
        signed int : 2;  /* Unnamed bitfield */
        signed int field1 : 5;
        union {
            struct {
                char arr[2][3];
            } a;
            struct {
                void *p;
            } b;
        } alt;
    } bits;
    int regular;
};

/* Level 4: Anonymous structs/unions */
struct Level4 {
    struct {  /* Anonymous struct */
        int x;
        union {  /* Anonymous union */
            float y;
            double z;
        };
    };
    int w;
};

/* Level 5: Designated initializers in type context */
struct DesignatedExample {
    struct {
        int first;
        int second[2];
    } pair;
    union {
        struct {
            char a;
            char b;
        } chars;
        short both;
    } combo;
};

/* Complex array declarations */
struct WithArrays {
    int simple[10];
    int multi[2][3][4];
    struct {
        char *names[5];
    } nested_array;
    int (*func_ptr_array[3])(void);
};

#endif /* DEEP_NESTED_STRUCTS_H */
