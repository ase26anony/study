/* Test file 1: Deeply nested structs and unions */
#ifndef NESTED_STRUCTS_C
#define NESTED_STRUCTS_C

/* Simple nested struct */
struct Level1 {
    int a;
    struct Level2 {
        char b;
        struct Level3 {
            long c;
            struct Level4 {
                float d;
                struct Level5 {
                    double e;
                } inner5;
            } inner4;
        } inner3;
    } inner2;
};

/* Union within struct within union */
union OuterUnion {
    int x;
    struct {
        char y;
        union {
            long z;
            struct {
                float w;
            } inner_struct;
        } inner_union;
    } inner_struct;
};

/* Array of nested structs */
struct ArrayHolder {
    struct Element {
        int id;
        struct Data {
            char name[20];
            struct Metadata {
                int version;
                struct {
                    unsigned int flags;
                } details;
            } meta;
        } data;
    } elements[10][5];  /* 2D array */
};

/* Bit-fields with nested anonymous struct */
struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 4;
    struct {
        unsigned int c : 2;
        unsigned int d : 6;
    } nested_bits;
    unsigned int e : 8;
};

/* Designated initializers with nesting */
struct InitExample {
    struct {
        int x;
        struct {
            int y;
            int z;
        } point;
    } coord;
    int values[3][2];
};

/* Function returning pointer to nested struct */
struct Outer* create_outer(void);

/* Pointer to array of pointers to structs */
struct Node** (*get_node_table(void))[10];

#endif /* NESTED_STRUCTS_C */
