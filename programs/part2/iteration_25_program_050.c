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

/* Level 3: Anonymous structs and unions */
struct DeepNest {
    struct {  /* Opening brace */
        union {
            struct {
                int x : 5;  /* Bit-field */
                int y : 3;
            } bits;
            long all;
        } u1;
        struct {
            char arr[3][4][5];  /* Triple nested array */
            struct {
                void (*callback)(int, char);  /* Function pointer */
            } handler;
        } s2;
    } anonymous;
};

/* Level 4: Mixed delimiters in initializers */
struct WithInitializer {
    int values[3][2];
    struct {
        char *name;
        int count;
    } info;
} global_var = {
    .values = { {1, 2}, {3, 4}, {5, 6} },  /* Nested braces */
    .info = { "test", 42 }
};

/* Level 5: Pointer to nested struct */
struct Outer {
    struct Middle {
        struct Inner {
            int data;
            int (*process)(struct Inner *self);  /* Self-referential */
        } *inner_ptr;
        void (*methods[5])(struct Middle *);  /* Array of function pointers */
    } mid;
    struct Middle *mid_array[10];  /* Array of pointers */
};

#endif /* NESTED_STRUCTS_H */
