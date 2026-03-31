/* Test file for gengtype parser - deeply nested structs and unions */

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

/* Level 2: Anonymous structs and unions */
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

/* Level 3: Arrays within nested structs */
struct Level3 {
    int matrix[3][4];
    struct {
        char *names[5];
        struct {
            int (*callback)(void);
            union {
                void (*vfunc)(int, char);
                int (*ifunc[2])(float);
            } funcs;
        } ops;
    } data;
};

/* Level 4: Bit-fields and designated initializers */
struct Level4 {
    unsigned int flags : 4;
    struct {
        int : 2;  /* unnamed bit-field */
        unsigned int mode : 3;
        struct {
            signed int value : 5;
            union {
                unsigned char : 4;
                unsigned char nibble : 4;
            };
        } bits;
    } state;
};

/* Level 5: Function pointers in nested structs */
struct Level5 {
    int (*(*complex_func)(int, char))(float);
    struct {
        void (*handlers[3])(struct Level5 *);
        union {
            char *(*str_func)(void);
            int (*int_func)(int, ...);
        } utils;
    } callbacks;
};

/* Level 6: Mixed nested types with all delimiters */
struct Level6 {
    int arr[2][3];
    struct {
        union {
            struct {
                int (*(*nested_callback)[5])(void);
            } s;
            void *ptr;
        } u;
        char name[];
    } flexible;
};

/* GCC attributes with nested parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data __attribute__((aligned(8)));
    struct {
        char buffer[64] __attribute__((aligned(32)));
    } __attribute__((packed)) inner;
};

/* Designated initializers with nested braces */
struct Level6 level6_instance = {
    .arr = { {1, 2, 3}, {4, 5, 6} },
    .flexible = {
        .u = { .s = { .nested_callback = NULL } },
        .name = "test"
    }
};

/* Array of nested structs */
struct Level1 level1_array[10] = {
    [0] = { .a = 1, .inner1 = { .b = 'x', .u1 = { .c = 100 } } },
    [5] = { .a = 2, .inner1 = { .b = 'y', .u1 = { .d = 200 } } }
};
