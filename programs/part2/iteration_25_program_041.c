/* Test file 1: Deeply nested structs and unions */
#ifndef DEEP_NESTED_STRUCTS_H
#define DEEP_NESTED_STRUCTS_H

/* Complex nested anonymous struct with union */
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
                } deeper;
            } nested;
        } u;
    } anonymous;
    
    /* Array with multiple dimensions */
    int multi_dim[3][4][5][2];
};

/* Struct with bit-fields and designated initializers */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int : 4;  /* Padding */
    struct {
        signed int value : 10;
        unsigned int : 6;
    } packed;
    
    /* Nested array in struct */
    char matrix[2][3];
};

/* Union containing struct containing union... */
union DeepUnion {
    struct {
        union {
            struct {
                int x;
                double y;
            } s1;
            union {
                char a;
                short b;
            } u2;
        } inner_union;
        long z;
    } outer_struct;
    float f_array[4];
};

/* Mix of all delimiters in type definition */
struct MixedDelimiters {
    /* Function pointer array */
    void (*callbacks[5])(int, char);
    
    /* Nested anonymous struct with initializer */
    struct {
        int (*comparator)(const void *, const void *);
        char name[50];
    } handler;
    
    /* Multi-dimensional array with designated initializer */
    int grid[2][3] = { [0] = {1, 2, 3}, {4, 5, 6} };
};

#endif /* DEEP_NESTED_STRUCTS_H */
