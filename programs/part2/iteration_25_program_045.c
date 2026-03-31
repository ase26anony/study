/* Test file 1: Deeply nested struct/union definitions */

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
            } inner_inner;
        } u;
    } inner;
};

/* Level 2: Anonymous structs and unions */
struct Level2 {
    struct {
        int x;
        union {
            char y;
            struct {
                int z[3];
            } s;
        };
    };
    long arr[2][3];
};

/* Level 3: Bit-fields and arrays */
struct Level3 {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    struct {
        signed int value : 4;
        unsigned int : 0;  /* Force alignment */
        struct {
            char bits : 2;
            char : 6;
        } packed;
    } bits;
    int matrix[2][3][4];
};

/* Level 4: Designated initializers in type context (GCC extension) */
struct Level4 {
    struct {
        int a;
        int b;
        int c;
    } values = { .a = 1, .b = 2, .c = 3 };
    union {
        int x;
        double y;
        struct {
            char z[10];
        };
    } data = { .x = 42 };
};

/* Level 5: Mixed nesting with all delimiters */
struct Level5 {
    struct {
        int (*func_ptr)(int, char);
        void (*arr_ptr[3])(void);
    } callbacks;
    union {
        struct {
            int matrix[2][2];
        } s;
        long flat[4];
    } data;
    struct {
        enum { RED, GREEN, BLUE } color;
        struct {
            int x, y;
        } coord;
    } state;
};
