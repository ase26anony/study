/* Test file for gengtype parser - deeply nested structs and unions */
#ifndef DEEP_NESTED_STRUCTS_C
#define DEEP_NESTED_STRUCTS_C

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
        char *strings[10];
        struct {
            int (*callbacks[5])(void);
            union {
                void (*void_func)(int, char);
                int (*int_func[2])(double);
            } func_union;
        } callback_container;
    } nested;
};

/* Level 3: Deeply nested anonymous structs/unions */
struct DeepNest {
    struct {
        union {
            struct {
                int a;
                struct {
                    char b;
                    union {
                        short c;
                        struct {
                            float d[2][3];
                            double (*e)[4];
                        } deepest;
                    } u2;
                } level4;
            } level3;
            long alternate;
        } anon_union;
    } anon_struct;
    int final;
};

/* Level 4: Mixed nesting with bitfields */
struct MixedBitfields {
    unsigned int flags : 3;
    struct {
        signed int a : 5;
        unsigned long b : 12;
        union {
            struct {
                int x : 2;
                int y : 6;
                int z : 8;
            } bits;
            unsigned int full;
        } packed_union;
    } bit_container;
    int regular_array[2][3];
};

/* Level 5: Designated initializers in type definitions (GCC extension) */
struct WithDesignators {
    struct {
        int a;
        int b;
        struct {
            char c;
            double d;
        } inner;
    } nested = { .a = 1, .inner = { .c = 'x', .d = 3.14 } };
    int arr[4] = { [0] = 1, [2] = 3, [3] = 4 };
};

#endif /* DEEP_NESTED_STRUCTS_C */
