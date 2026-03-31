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
struct MatrixContainer {
    struct {
        int rows;
        int cols;
        int data[10][20];
        struct {
            int (*row_processor)(int[20]);
            void (*col_processor)(int, int);
        } processors;
    } matrix;
};

/* Level 3: Anonymous structs and unions */
struct AnonymousNest {
    struct {
        union {
            struct {
                int x;
                int y;
            } point;
            struct {
                float r;
                float theta;
            } polar;
        } coord;
        int flags : 4;
        int : 4; /* unnamed bitfield */
        unsigned int mode : 8;
    } data;
    struct {
        void (*callback)(struct AnonymousNest *);
        int (*validator)(const struct AnonymousNest[5]);
    } ops;
};

/* Level 4: Designated initializers in type context */
struct DesignatedInit {
    struct {
        int first __attribute__((aligned(16)));
        int second;
    } pair;
    union {
        struct {
            char a;
            char b;
            char c;
            char d;
        } chars;
        int value;
    } data __attribute__((packed));
    int array[3] __attribute__((aligned(32)));
};

/* Level 5: Function pointers returning struct pointers */
struct ComplexFuncs {
    struct Nested {
        int value;
        struct Deeper {
            char *name;
            int (*getter)(void);
        } deeper;
    } *(*factory)(int count);
    
    void (*(*callback_chain)(int))(struct ComplexFuncs *);
    
    struct {
        int (*comparator)(const void *, const void *);
        void (*destructor)(void *);
    } vtable[10];
};

/* Level 6: Mixed nested with all delimiters */
struct UltimateNest {
    /* Parentheses in function pointer */
    int (*(*func_ptr_array[5])(int, char))[10];
    
    /* Brackets in array */
    struct {
        int matrix[3][2][4];
        void (*handlers[2][3])(int, float);
    } container;
    
    /* Braces in anonymous struct */
    union {
        struct {
            int a;
            int b;
        };
        struct {
            long x;
            long y;
        };
    } coords;
    
    /* All three together */
    void (*(*complex[2])(int[3]))(float, double) __attribute__((deprecated));
};

/* Level 7: Recursive structure */
struct TreeNode {
    int value;
    struct TreeNode *children[4];
    struct {
        struct TreeNode *(*allocator)(void);
        void (*deallocator)(struct TreeNode *);
    } memory_ops;
};

/* Level 8: With attributes containing nested parentheses */
struct Attributed {
    int data __attribute__((aligned(64), packed, deprecated("use new_data instead")));
    struct {
        char *name __attribute__((nonnull(1, 2), format(printf, 2, 3)));
        int id;
    } info;
    void (*api[3])(void) __attribute__((noreturn));
};

/* Level 9: Nested in typedef */
typedef struct {
    struct Inner {
        union {
            int i;
            float f;
            struct {
                char c;
                short s;
            } cs;
        } data;
        int (*processor)(union { int a; float b; } param);
    } inner;
} DeepTypedef;

/* Level 10: Maximum nesting */
struct FinalChallenge {
    struct A {
        struct B {
            struct C {
                struct D {
                    struct E {
                        int value;
                        struct F {
                            struct G {
                                struct H {
                                    struct I {
                                        struct J {
                                            int deepest;
                                        } j;
                                    } i;
                                } h;
                            } g;
                        } f;
                    } e;
                } d;
            } c;
        } b;
    } a;
    
    /* Function pointer with deep nesting */
    int (*(*(*(*nested_func_ptr)(void))[5])(int))(char);
    
    /* Array with nested initializer */
    struct {
        int arr[2][3][4];
    } arrays[5][6];
};
