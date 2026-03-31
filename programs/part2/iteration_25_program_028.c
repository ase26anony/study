/* Test file 1: Deeply nested structs and unions */

/* Level 1 */
struct Level1 {
    int a;
    /* Level 2 */
    struct {
        char b;
        /* Level 3 */
        union {
            long c;
            /* Level 4 */
            struct {
                float d;
                /* Level 5 */
                union {
                    double e;
                    /* Level 6 */
                    struct {
                        short f;
                        /* Level 7 */
                        struct {
                            int g;
                            /* Level 8 */
                            union {
                                char h;
                                long i;
                            } u8;
                        } s7;
                    } s6;
                } u5;
            } s4;
        } u3;
    } s2;
    int arr1[5][7];
};

/* Mixed delimiters in array initialization */
struct MixedInit {
    int matrix[3][2][4];
    void (*callbacks[5])(int, char);
} mixed = {
    .matrix = { 
        [0] = { {1, 2, 3, 4}, {5, 6, 7, 8} },
        { {9, 10, 11, 12}, {13, 14, 15, 16} },
        { {17, 18, 19, 20}, {21, 22, 23, 24} }
    },
    .callbacks = { NULL, NULL, NULL, NULL, NULL }
};

/* Anonymous structs and unions */
struct AnonymousContainer {
    struct {
        int x;
        union {
            char y;
            struct {
                float z[2][3];
            } inner;
        } u;
    } anon;
    int (*func_ptr_array[3])(struct { int a; double b; } param);
};

/* Bit-fields with complex types */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int : 4; /* Padding */
    unsigned int flags[2] : 8;
    struct {
        int nested : 3;
    } inner;
};

/* Designated initializers with nesting */
struct Designated {
    int a;
    struct {
        int b[2][2];
        union {
            char c;
            long d;
        } u;
    } inner;
} designated = {
    .a = 42,
    .inner = {
        .b = { {1, 2}, {3, 4} },
        .u = { .d = 0xDEADBEEF }
    }
};
