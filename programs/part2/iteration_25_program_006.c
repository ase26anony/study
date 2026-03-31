/* Test file for gengtype parser - nested structs/unions */
#include <stddef.h>

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

/* Level 2: Anonymous struct/union */
struct Level2 {
    struct {
        int x;
        union {
            float f;
            struct {
                double d;
                char c;
            } s;
        };
    };
    int y;
};

/* Level 3: Deep nesting with arrays */
struct Level3 {
    struct A {
        int a[5];
        struct B {
            char b[3][2];
            union C {
                long l;
                struct D {
                    short s;
                    int i;
                } d;
            } c[2];
        } b;
    } a;
    int final;
};

/* Level 4: Bit-fields and designated initializers */
struct Level4 {
    unsigned int flags : 4;
    signed int value : 12;
    struct {
        struct {
            int a : 2;
            int b : 3;
            int c : 5;
        } bits;
        union {
            char arr[4];
            struct {
                char x, y, z, w;
            } chars;
        } data;
    } nested;
};

/* Level 5: Function pointers inside structs */
struct Level5 {
    int (*callback)(int, char);
    struct {
        void (*start)(void);
        int (*process)(struct Level5 *self, float);
        void (*end)(int status);
    } ops;
};

/* Level 6: Mixed everything */
struct Level6 {
    /* Array of structs containing unions */
    struct {
        union {
            int i;
            float f;
            struct {
                char c;
                short s;
            } cs;
        } data;
        int tag;
    } items[10];
    
    /* Pointer to array of function pointers */
    int (*(*complex_ptr)[5])(char, float);
    
    /* Nested anonymous struct with bitfields */
    struct {
        unsigned int a : 1;
        unsigned int b : 2;
        unsigned int c : 3;
        struct {
            int x : 4;
            int y : 4;
        } xy;
    };
};

/* Global variables with complex initializers */
struct Level1 g_level1 = { 
    .a = 42,
    .inner1 = {
        .b = 'X',
        .u1 = { .c = 123 }
    }
};

struct Level3 g_level3 = {
    .a = {
        .a = {1, 2, 3, 4, 5},
        .b = {
            .b = {{'a', 'b'}, {'c', 'd'}, {'e', 'f'}},
            .c = {
                [0] = { .l = 1000 },
                [1] = { .d = { .s = 50, .i = 100 } }
            }
        }
    },
    .final = -1
};

/* Array with nested designated initializers */
int g_matrix[3][2][2] = { 
    [0] = { {1, 2}, {3, 4} },
    [1] = { {5, 6}, {7, 8} },
    [2] = { {9, 10}, {11, 12} }
};

/* String array with braces */
const char *g_strings[][2] = {
    {"hello", "world"},
    {"foo", "bar"},
    {"test", "case"}
};
