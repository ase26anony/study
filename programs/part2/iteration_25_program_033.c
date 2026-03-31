/* Test file for gengtype parser - deeply nested structs and unions */

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
        } u1;
    } inner;
};

/* Level 2: Array within nested struct */
struct Level2 {
    struct {
        int matrix[3][4];
        union {
            char str[10];
            void *ptr;
            struct {
                int (*callback)(int, char);
                float data;
            } func_data;
        } data_union;
    } container;
    long id;
};

/* Level 3: Anonymous structs and unions */
struct Level3 {
    struct {
        union {
            struct {
                int x : 5;
                int y : 3;
                int z : 8;
            } bits;
            unsigned int raw;
        };
        double values[2][2];
    } anonymous_member;
    
    /* Nested with designated initializers syntax */
    struct Config {
        int mode;
        struct {
            int min;
            int max;
            int (*validator)(int);
        } range;
        union {
            struct {
                char *name;
                int (*handler)(void);
            } named;
            void *opaque;
        } data;
    } config;
};

/* Level 4: Multiple levels of nesting */
struct Level4 {
    struct A {
        struct B {
            union C {
                struct D {
                    int (*complex_func)(int (*(*)(void))[5], char);
                    struct E {
                        float arr[7][8];
                        struct F {
                            short s;
                            long l;
                            union {
                                int i;
                                double d;
                            } choice;
                        } f;
                    } e;
                } d;
                void *alternate;
            } c;
        } b;
    } a;
};

/* Mix of all delimiters in initializers */
struct MixedDelimiters {
    int (*func_ptr_array[3])(int, char);
    struct {
        int matrix[2][3];
        union {
            char str[20];
            int (*callbacks[2])(void);
        } u;
    } data;
};

/* Global variable with complex initializer */
struct MixedDelimiters global_var = {
    .func_ptr_array = { NULL, NULL, NULL },
    .data = {
        .matrix = { {1, 2, 3}, {4, 5, 6} },
        .u = {
            .str = "test"
        }
    }
};

/* Function returning nested struct pointer */
struct Level1 *(*get_level1_factory(void))(int) {
    /* Implementation not needed for parsing */
    return NULL;
}
