/* Test file 1: Deeply nested structs and unions */
#ifndef DEEP_NESTED_STRUCTS_H
#define DEEP_NESTED_STRUCTS_H

/* Level 1: Basic nested struct */
struct Level1 {
    int a;
    struct {
        char b;
        union {
            int c;
            long d;
            struct {
                short e;
                union {
                    char f;
                    double g;
                } inner_union;
            } deep_struct;
        } level2_union;
    } level2_struct;
    float h;
};

/* Level 2: Arrays within nested structs */
struct ArrayNest {
    int matrix[3][4][5];
    struct {
        char *strings[10];
        union {
            int (*func_ptrs[5])(void);
            struct {
                void (*callback[3])(int, char);
            } callback_struct;
        } func_union;
    } nested_with_arrays;
};

/* Level 3: Bit-fields and anonymous unions */
struct BitFieldNest {
    unsigned int flags : 4;
    struct {
        unsigned char : 2;
        unsigned char mode : 3;
        union {
            struct {
                unsigned int : 1;
                unsigned int type : 2;
            } bit_struct;
            unsigned char raw;
        } anon_union;
    } control;
    int regular;
};

/* Level 4: Designated initializers in type definitions */
struct DesignatedInit {
    struct {
        int x;
        int y;
        struct {
            int z[2];
            union {
                char a;
                int b;
            } u;
        } point3d;
    } coord = { .x = 1, .y = 2, .point3d = { .z = {3, 4}, .u = { .b = 5 } } };
    int arr[2][3] = { [0] = {1, 2, 3}, {4, 5, 6} };
};

/* Level 5: Multiple levels of nesting */
struct UltimateNest {
    struct A {
        union B {
            struct C {
                int (*func_ptr)(struct D {
                    int x;
                    struct E {
                        union F {
                            char c;
                            long l;
                        } f;
                    } e;
                } *d);
            } c;
            void *ptr;
        } b;
    } a;
    int final;
};

#endif /* DEEP_NESTED_STRUCTS_H */
