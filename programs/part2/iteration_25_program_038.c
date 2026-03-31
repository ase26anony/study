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
struct WithArrays {
    int matrix[3][4][5];
    struct {
        char *names[10];
        struct {
            void *pointers[7];
            union {
                int (*func_ptrs[5])(void);
                char (*char_ptrs[3])[8];
            } func_union;
        } ptr_struct;
    } nested;
};

/* Level 3: Anonymous structs and unions */
struct AnonymousNesting {
    struct {
        union {
            struct {
                int x : 3;
                int y : 5;
                int z : 8;
            } bits;
            unsigned int raw;
        } data;
        struct {
            char a;
            struct {
                short b;
                struct {
                    int c;
                } triple_nested;
            } double_nested;
        } chars;
    } anonymous;
};

/* Level 4: Designated initializers in type context */
struct WithDesignators {
    struct {
        int values[4];
        union {
            struct {
                float x, y;
            } point;
            struct {
                int start, end;
            } range;
        } data;
    } container[2] = {
        [0] = {
            .values = {1, 2, 3, 4},
            .data = {
                .point = {1.0f, 2.0f}
            }
        },
        [1] = {
            .values = {5, 6, 7, 8},
            .data = {
                .range = {100, 200}
            }
        }
    };
};

/* Level 5: Mixed delimiters in single declaration */
struct MixedDelimiters {
    /* Array of pointers to functions returning pointers to arrays */
    int (*(*callbacks[3])(int, char))[5];
    
    /* Function pointer with complex parameters */
    void (*(*signal_handler)(int signo, void (*old_handler)(int)))(int);
    
    /* Nested array with function pointers */
    struct {
        char (*(*string_funcs[2])(void))[10];
        int (*math_funcs[3])(float, double);
    } func_groups;
};

/* Level 6: GCC attributes with nested parentheses */
struct __attribute__((aligned(32), packed, 
    deprecated("Use NewStruct instead"))) AttributedStruct {
    int field1 __attribute__((mode(byte)));
    long field2 __attribute__((aligned(16)));
    
    struct {
        char data[64];
    } buffer __attribute__((packed));
};

/* Level 7: Deeply nested bit-fields */
struct DeepBitfields {
    struct {
        struct {
            struct {
                unsigned a : 1;
                unsigned b : 2;
                unsigned c : 3;
                unsigned d : 4;
                struct {
                    unsigned e : 1;
                    unsigned f : 1;
                } inner_bits;
            } level3;
            int padding;
        } level2;
        char filler[8];
    } level1;
};

/* Level 8: Union with anonymous structs */
union ComplexUnion {
    struct {
        int type;
        union {
            struct {
                int x, y;
            } point;
            struct {
                float radius;
                int sides;
            } circle;
            struct {
                char *text;
                int length;
            } string;
        } shape;
    } tagged;
    
    unsigned char raw[32];
    
    struct {
        long id;
        void *ptr;
    } metadata;
};

/* Level 9: Array of nested structs with initializers */
struct ArrayOfNested {
    struct Element {
        int id;
        struct {
            char name[20];
            struct {
                float values[3];
                int count;
            } data;
        } info;
    } elements[5] = {
        {1, {"first", {{1.0, 2.0, 3.0}, 3}}},
        {2, {"second", {{4.0, 5.0, 6.0}, 3}}},
        {3, {"third", {{7.0, 8.0, 9.0}, 3}}}
    };
};

/* Level 10: The ultimate nested challenge */
struct UltimateNesting {
    struct A {
        union B {
            struct C {
                int (*(*func_ptr_array[2][3])(struct D {
                    int x;
                    struct E {
                        char y;
                        union F {
                            short z;
                            long w;
                        } u;
                    } inner;
                } param))[4][5];
                
                struct G {
                    char data[10];
                } arrays[6][7];
            } c_struct;
            
            void *generic_ptr;
        } b_union;
        
        int simple;
    } a_struct;
    
    /* Mix all delimiters in one declaration */
    void (*(*mixed[2])(int, char (*)[3]))(float, double (*)(long));
};
