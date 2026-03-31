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
        } u;
    } inner;
};

/* Level 2: Array within nested struct */
struct ComplexArray {
    int matrix[3][4][5];
    struct {
        char *names[10][20];
        union {
            int (*func_ptrs[5])(void);
            struct {
                void (*callbacks[3][2])(int, char);
            } cb_struct;
        } u_array;
    } nested;
};

/* Level 3: Anonymous structs and unions */
struct AnonymousNest {
    struct {
        union {
            struct {
                int x;
                struct {
                    char y;
                    union {
                        short z;
                        long w;
                    } deepest;
                } mid;
            } a;
            struct {
                double values[4][3];
            } b;
        };
        int tag;
    };
    float extra;
};

/* Level 4: Bit-fields with nested structs */
struct BitFieldNest {
    unsigned int flags : 4;
    struct {
        unsigned int a : 2;
        unsigned int b : 3;
        struct {
            unsigned int c : 1;
            unsigned int d : 5;
        } bits;
    } packed;
    int regular;
};

/* Level 5: Designated initializers in type context */
struct WithDesignators {
    struct {
        int first;
        union {
            char alpha;
            struct {
                int beta[3];
                double gamma;
            } greek;
        } letters;
    } start;
    int end;
};

/* Mixed delimiters in single declaration */
struct AllDelimiters {
    int (*func_array[5])(char[10], struct {int x; int y;});  /* (), [], {} */
    union {
        struct {
            void (*callback)(int (*)(char), double[3][4]);
        } cb;
        int matrix[2][3][4];
    } data;
};
