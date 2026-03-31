/* Test file 1: Deeply nested structs and unions */
#ifndef NESTED_STRUCTS_H
#define NESTED_STRUCTS_H

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
        char *names[10][20];
        union {
            int (*callback[5])(void);
            struct {
                void (*handlers[3])(int, char);
            } handler_struct;
        } func_union;
    } data;
};

/* Level 3: Deep nesting with bitfields */
struct DeepNested {
    unsigned int flags : 4;
    struct {
        int : 16; /* unnamed bitfield */
        struct {
            union {
                struct {
                    char a : 2;
                    char b : 3;
                    char c : 3;
                } bits;
                unsigned char byte;
            } bit_union;
            int arr[2][3];
        } deeper;
    } container;
};

/* Level 4: Mixed anonymous structs/unions */
struct MixedAnonymous {
    struct {
        int x;
        union {
            float y;
            double z;
        };
    };
    union {
        char id[20];
        struct {
            short s1, s2;
        };
    };
    int values[5];
};

/* Level 5: Designated initializers in type context (GCC extension) */
struct WithDesignators {
    struct {
        int field1;
        int field2;
    } s1;
    int array[5];
} my_struct = {
    .s1 = {
        .field1 = 10,
        .field2 = 20
    },
    .array = { [0] = 1, [2] = 3, [4] = 5 }
};

#endif /* NESTED_STRUCTS_H */
