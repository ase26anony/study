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
    } inner;
};

/* Level 2: Anonymous structs and unions */
struct OuterContainer {
    struct {
        union {
            int x;
            struct {
                char y;
                struct {
                    unsigned z: 4;
                    unsigned w: 12;
                } bits;
            } nested;
        } anon_union;
    } anon_struct;
    
    int arr[3][4][5];
};

/* Level 3: Mixed nesting with arrays */
struct ComplexType {
    struct {
        union {
            void *ptr;
            struct {
                int (*callback)(int, char);
                char buffer[100];
            } data;
        } variant;
    } container;
    
    struct {
        int matrix[2][3];
        struct {
            float coords[3][3][3];
        } spatial;
    } arrays;
};

/* Level 4: Designated initializers in type context */
struct WithInitializer {
    int a;
    struct {
        int b;
        union {
            int c;
            struct {
                int d[2][2];
            } inner;
        } u;
    } nested;
};

/* Array of nested structs */
struct NestedArray {
    struct Element {
        union {
            int val;
            struct {
                char str[10];
            } text;
        } data;
        struct Element *next;
    } elements[50];
};

#endif /* NESTED_STRUCTS_H */
