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

/* Level 2: Array within nested struct */
struct MatrixWrapper {
    struct {
        int rows;
        int cols;
        int data[10][20];
    } matrix;
    union {
        struct {
            float (*transform)[4][4];
        } transform_data;
        void (*callback)(int, char);
    } meta;
};

/* Level 3: Anonymous structs and unions */
struct AnonymousContainer {
    struct {
        union {
            struct {
                int x:4;
                int y:4;
                int z:8;
            } bits;
            unsigned int value;
        };
        char name[50];
    } component;
    
    struct {
        int (*compare)(const void *, const void *);
        void (*print)(struct AnonymousContainer *);
    } operations;
};

/* Level 4: Designated initializers in type definitions */
struct DesignatedExample {
    struct {
        int a;
        int b;
        int c;
    } points[3];
    
    union {
        struct {
            int mode:2;
            int flags:6;
        } settings;
        unsigned char config;
    };
} global_var = {
    .points = {
        [0] = { .a = 1, .b = 2, .c = 3 },
        { .a = 4, .b = 5 },
        [2] = { .c = 6 }
    },
    .settings = { .mode = 1, .flags = 0x3F }
};

/* Level 5: Mixed nesting with all delimiters */
struct UltimateNest {
    int (*(*complex_array[5])(void))[10];
    struct {
        union {
            void (*(*signal_handler)(int, void *))(int);
            struct {
                char buffer[256][128];
                int (*processor)(char (*)[128]);
            } data;
        } u;
    } nested;
};
