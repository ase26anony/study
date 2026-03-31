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
            int (*compare)(const void *, const void *);  /* Function pointer */
        } s;
        void *ptr;
    } metadata;
};

/* Level 3: Anonymous structs/unions with bitfields */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    struct {
        unsigned int : 4;  /* Unnamed bitfield */
        unsigned int nibble : 4;
        union {
            struct {
                unsigned int lower : 8;
                unsigned int upper : 8;
            } bytes;
            unsigned short word;
        } value;
    } control;
    int array[3][4][5];  /* Triple nested array */
};

/* Level 4: Designated initializers (in type definition context) */
struct DesignatedInit {
    int x;
    struct {
        int a;
        int b;
        int c;
    } coords;
    union {
        struct {
            float r;
            float g;
            float b;
            float a;
        } rgba;
        unsigned int packed;
    } color;
};

/* Level 5: Mixed nested with function pointers */
struct CallbackSystem {
    void (*simple_cb)(void);
    int (*complex_cb)(int, char *);
    struct {
        void (*start)(struct CallbackSystem *);
        int (*process)(int (*)(int), void *);
        void (*cleanup)(void);
    } ops;
    union {
        struct {
            int (*filter)(int[10][10]);  /* Array parameter */
            void (*transform)(float (*)[5]);
        } image_ops;
        struct {
            char *(*alloc)(size_t);
            void (*free)(void *);
        } mem_ops;
    } specialized;
};

#endif /* NESTED_STRUCTS_H */
