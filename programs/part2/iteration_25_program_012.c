/* Test file for gengtype - deeply nested structs and unions */

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
        char *names[10];
        struct {
            void *data;
            size_t size;
        } metadata[2];
    } header;
};

/* Level 3: Anonymous structs and unions */
struct AnonymousNesting {
    struct {
        union {
            struct {
                int x:4;
                int y:12;
            } bits;
            unsigned int raw;
        } data;
    };
    union {
        struct {
            float f;
            double d;
        };
        long double ld;
    } value;
};

/* Level 4: Designated initializers in type definitions */
struct WithInitializer {
    struct {
        int a;
        int b;
        struct {
            char c;
            short d;
        } nested;
    } inner = { .a = 1, .b = 2, .nested = { .c = 'x', .d = 3 } };
    int arr[2][3] = { [0] = {1, 2, 3}, [1] = {4, 5, 6} };
};

/* Level 5: Bit-fields with complex nesting */
struct BitFieldNest {
    unsigned int flags:8;
    struct {
        unsigned int a:1;
        unsigned int b:2;
        unsigned int c:3;
        union {
            unsigned int d:4;
            struct {
                unsigned int e:2;
                unsigned int f:2;
            };
        };
    } control;
};

/* Level 6: Mixed delimiters in single declaration */
struct MixedDelimiters {
    void (*callback)(int, char);  /* Function pointer */
    int (*matrix_ptr)[5][10];     /* Pointer to 2D array */
    struct {
        union {
            char (*string_array[5])[20];  /* Array of pointers to arrays */
            void *generic;
        };
    } container;
};

/* Level 7: Recursive structure (self-referential) */
struct TreeNode {
    int value;
    struct TreeNode *left;
    struct TreeNode *right;
    struct {
        struct TreeNode *parent;
        int depth;
    } metadata;
};

/* Level 8: Union of structs containing arrays */
union UnionOfStructs {
    struct {
        int type;
        char name[50];
        struct {
            int x, y;
        } coords[100];
    } data;
    struct {
        float values[25];
        struct {
            double min, max;
        } range;
    } stats;
};

/* Level 9: Nested with function pointers */
struct WithFunctionPointers {
    int (*comparator)(const void *, const void *);
    struct {
        void (*init)(void);
        void (*cleanup)(void);
        struct {
            int (*validate)(int, char **);
            void (*log)(const char *, ...);
        } helpers;
    } ops;
};

/* Level 10: Maximum nesting challenge */
struct UltimateNest {
    struct {
        union {
            struct {
                int a[2][3][4];
                struct {
                    char b[10];
                    struct {
                        short c;
                        struct {
                            long d;
                            union {
                                float e;
                                double f;
                                struct {
                                    void *g;
                                    size_t h;
                                };
                            };
                        };
                    };
                };
            };
            void *ptr;
        };
    } data[5];
};
