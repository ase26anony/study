/* Test file 1: Deeply nested structs and unions */
/* This file contains multiple levels of nested delimiters */

/* Level 1: Basic nested struct */
struct Level1 {
    int a;
    struct {
        char b;
        union {
            short c;
            long d;
        } u1;
    } inner1;
};

/* Level 2: Array within nested struct */
struct Level2 {
    struct {
        int matrix[3][4];
        union {
            float f[2][2];
            double d[2];
        } u2;
    } data;
    struct {
        struct {
            char *name;
        } metadata;
    } info;
};

/* Level 3: Deep nesting with anonymous structs/unions */
struct Level3 {
    struct {
        union {
            struct {
                int x;
                struct {
                    char y;
                    union {
                        short z;
                        long w;
                    } deep_union;
                } deeper;
            } s1;
            struct {
                float arr[2][3][4];
            } s2;
        } u3;
    } container;
};

/* Level 4: Mixed delimiters in single declaration */
struct Level4 {
    /* Function pointer array */
    void (*callbacks[5])(int, char);
    
    /* Nested struct with array of structs */
    struct {
        struct {
            int id;
            char name[50];
        } items[10];
    } collection;
    
    /* Union with nested arrays */
    union {
        int (*int_funcs[3])(void);
        void (*void_funcs[2])(int, float);
    } func_union;
};

/* Level 5: Extremely deep nesting */
struct Level5 {
    struct A {
        union B {
            struct C {
                int (*complex_ptr)(char (*)[10]);
                struct D {
                    union E {
                        struct F {
                            int matrix[2][2][2][2];
                        } f;
                    } e;
                } d;
            } c;
        } b;
    } a;
};

/* Global variables using these types */
struct Level1 global_l1 = { 
    1, 
    { 'b', { 100 } } 
};

struct Level2 global_l2 = {
    { 
        { {1,2,3,4}, {5,6,7,8}, {9,10,11,12} },
        { .f = { {1.0, 2.0}, {3.0, 4.0} } }
    },
    { { "test" } }
};

/* Array of complex structs */
struct Level3 level3_array[5] = {0};

/* Pointer to nested struct */
struct Level4 *level4_ptr = 0;

/* Multi-dimensional array of pointers */
struct Level5 *level5_matrix[2][3] = {0};
