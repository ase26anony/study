/* File: nested_structs.h
 * Purpose: Exercise gengtype parser with deeply nested struct/union definitions
 */

#ifndef NESTED_STRUCTS_H
#define NESTED_STRUCTS_H

/* Level 1: Basic nested struct */
struct Outer1 {
    struct {
        int a;
        union {
            char b;
            long c;
            struct {
                short d;
                unsigned e;
            } inner_inner;
        } u;
    } inner;
    int arr[5][7];
};

/* Level 2: Anonymous structs and unions */
struct Outer2 {
    struct {  /* Anonymous struct */
        union {  /* Anonymous union */
            struct {  /* Nested anonymous struct */
                float x;
                double y;
            };
            long z;
        };
        int w;
    };
    char multi_array[3][4][5];
};

/* Level 3: Bit-fields with complex nesting */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    struct {
        unsigned int nested_flag1 : 3;
        unsigned int : 4;  /* Unnamed bit-field */
        union {
            unsigned int union_bit1 : 5;
            unsigned int union_bit2 : 6;
        } bit_union;
    } nested_bits;
    int regular_member;
};

/* Level 4: Designated initializers in type definitions (GCC extension) */
struct DesignatedInit {
    int a;
    struct {
        int b;
        int c;
    } inner;
    int array[4];
} designated_var = {
    .a = 1,
    .inner = {
        .b = 2,
        .c = 3
    },
    .array = {[0] = 10, [3] = 40}
};

/* Level 5: Mixed nested types with arrays of structs */
struct ComplexNesting {
    struct Level1 {
        union Level2 {
            struct Level3 {
                int a;
                struct Level4 {
                    char b;
                    union Level5 {
                        short c;
                        long d;
                    } level5_union;
                } level4_struct;
            } level3_struct;
            float level2_float;
        } level2_union;
        double level1_double;
    } level1_struct[2][3];
    
    struct {
        int (*func_ptr_array[4])(void);
    } anonymous_container;
};

#endif /* NESTED_STRUCTS_H */
