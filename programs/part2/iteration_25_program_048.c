/* Test file for gengtype parser - nested structs/unions */
#ifndef DEEP_NESTED_STRUCTS_H
#define DEEP_NESTED_STRUCTS_H

/* Struct with deeply nested anonymous unions and structs */
struct Level1 {
    struct {
        int a;
        union {
            char b;
            struct {
                long c;
                union {
                    short d;
                    struct {
                        int e;
                        float f;
                    } inner;
                } deep_union;
            } deeper;
        } u;
    } nested;
    
    /* Array with multiple dimensions */
    int arr[5][7][3];
    
    /* Pointer to array of pointers */
    int *(*(*complex_ptr)[10])[5];
};

/* Union with nested struct containing arrays */
union ContainerUnion {
    struct Data {
        char id[32];
        struct Metadata {
            int version;
            struct {
                unsigned int flags : 4;
                unsigned int type : 3;
            } bits;
            double values[8][2];
        } meta;
    } data;
    
    long raw[16];
};

/* Struct with designated initializers in type definition context */
struct WithInitializers {
    struct Point {
        int x, y, z;
    } points[4];
    
    union Choice {
        int ival;
        double dval;
        char sval[16];
    } choices[2];
    
    /* Nested anonymous struct */
    struct {
        int counter;
        struct {
            char tag;
            union {
                int num;
                float flt;
            } value;
        } entry;
    } state;
};

/* Bit-field extravaganza */
struct BitFieldStruct {
    unsigned int a : 1;
    unsigned int b : 2;
    struct {
        unsigned int c : 3;
        unsigned int d : 4;
        struct {
            unsigned int e : 1;
            unsigned int f : 2;
        } nested_bits;
    } inner_bits;
    
    /* Array of structs with bit-fields */
    struct {
        unsigned int flag : 1;
        unsigned int value : 7;
    } flags[8];
};

#endif /* DEEP_NESTED_STRUCTS_H */
