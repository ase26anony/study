/* Test file for deeply nested struct/union definitions */
/* This should trigger many '{' and '}' case statements */

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
            size_t size;
        } metadata[2];
    } container;
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
                struct {
                    double angle;
                    char quadrant;
                } polar;
            } circle;
        } shape;
    } geometry;
    
    /* Mix of all delimiters in bit-fields */
    struct {
        unsigned int flags : 4;
        struct {
            unsigned int : 2;
            unsigned int mode : 3;
            union {
                unsigned int raw : 8;
                struct {
                    unsigned int a : 2;
                    unsigned int b : 2;
                    unsigned int c : 2;
                    unsigned int d : 2;
                } parts;
            } data;
        } control;
    } bits;
};

/* Level 4: Designated initializers with nested braces */
#ifdef TEST_INITIALIZERS
struct WithInitializer = {
    .matrix = { [0] = {1, 2, 3}, {4, 5, 6}, {7, 8, 9} },
    .container = {
        .names = {"a", "b", "c"},
        .metadata = {
            [0] = {.data = NULL, .size = 0},
            {.data = (void*)0x1000, .size = 256}
        }
    }
};
#endif

/* Level 5: Function pointers inside structs */
struct WithCallbacks {
    void (*simple_cb)(int);
    int (*complex_cb)(struct Level1*, struct ComplexArray**);
    void (*nested_cb)(int (*)(char), float);
};

/* Mix of all delimiters in a single declaration */
struct UltimateMix {
    /* Array of pointers to functions returning pointers to arrays */
    int (*(*func_array[5])(void))[10];
    
    /* Nested anonymous union with bitfields */
    union {
        struct {
            unsigned int a : 3;
            unsigned int b : 5;
        } bits;
        unsigned char bytes[2];
    } packed_data;
    
    /* Pointer to array of structs */
    struct Level1 (*ptr_to_array)[20];
    
    /* Function returning pointer to function */
    void (*(*get_callback(int id))(int, char)) (void);
};

/* GCC attributes with nested parentheses */
struct __attribute__((aligned(32), packed, 
    deprecated("Use NewType instead"))) AttributedStruct {
    int data __attribute__((aligned(16)));
    char buffer[64] __attribute__((aligned(8)));
};

/* Forward declarations that also need balancing */
struct ForwardRef;
typedef struct ForwardRef* (*FactoryFunc)(struct ComplexArray (*)[10]);
