/* Test file for deeply nested struct/union definitions */
/* This will trigger many '{' and '}' cases in gengtype */

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
    } nested;
};

/* Level 2: Array within nested struct */
struct ComplexArray {
    int matrix[3][4][5];
    struct {
        char *names[10];
        struct {
            void *data;
            int count;
        } metadata;
    } info;
};

/* Level 3: Anonymous structs and unions */
struct AnonymousNesting {
    struct {
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
    } geometry;
    
    /* Bit-fields with nesting */
    struct {
        unsigned int flag1 : 1;
        unsigned int flag2 : 2;
        struct {
            unsigned int nested_flag1 : 3;
            unsigned int nested_flag2 : 4;
        } bits;
    } flags;
};

/* Level 4: Designated initializers in type definitions */
struct WithInitializer {
    int values[4];
    struct {
        char *name;
        int id;
    } item;
} global_var = {
    .values = {[0] = 1, [2] = 3, [3] = 4},
    .item = {
        .name = "test",
        .id = 42
    }
};

/* Level 5: Multiple levels of nesting */
struct OuterMost {
    struct Middle {
        struct InnerMost {
            int data;
            struct {
                char tag;
                union {
                    int num;
                    char str[20];
                    struct {
                        float x, y, z;
                    } vector;
                } value;
            } payload;
        } core;
        int count;
    } container;
    double timestamp;
};

/* Mixed delimiters in single declaration */
struct MixedDelimiters {
    int (*callback)(int, char);  /* () then () */
    void (*handlers[5])(void);   /* [] then () */
    struct {
        int (*nested_cb)(struct MixedDelimiters *);
    } ops;
};
