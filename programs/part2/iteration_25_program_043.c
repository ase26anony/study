/* Test file 1: Deeply nested structs and unions */
#ifndef DEEP_NESTED_STRUCTS_H
#define DEEP_NESTED_STRUCTS_H

/* Complex nested structure with all delimiter types */
struct Level1 {
    struct {
        int a;
        union {
            char b;
            struct {
                long c[3][2];
                union {
                    short d;
                    struct {
                        float e;
                        double f;
                    } inner;
                } u;
            } s;
        } nested_union;
    } anonymous_struct;
    
    /* Array with nested braces in initializer */
    int matrix[2][3] = { {1, 2, 3}, {4, 5, 6} };
    
    /* Function pointer member */
    void (*callback)(struct Level1 *);
};

/* Even deeper nesting */
struct OuterMost {
    struct {
        union {
            struct {
                struct Level1 level1;
                struct {
                    int (*func_ptr_array[3])(char, short);
                    union {
                        struct OuterMost *self_ptr;
                        void (*method)(struct OuterMost *);
                    } ptr_union;
                } inner_struct;
            } deep;
            long long data[4][2];
        } mega_union;
        
        /* Nested array with designated initializers */
        struct {
            int x;
            int y;
        } points[2] = { [0] = {.x = 1, .y = 2}, [1] = {.x = 3, .y = 4} };
    } container;
    
    /* Bit-fields with complex layout */
    struct {
        unsigned int flag1 : 1;
        unsigned int flag2 : 2;
        unsigned int : 3; /* Padding */
        unsigned int flag3 : 4;
    } flags;
};

/* Union with anonymous structs */
union ComplexUnion {
    struct {
        int type;
        union {
            char str[10];
            struct {
                int num;
                float fnum;
            } nums;
        } data;
    } tagged;
    
    long raw[5];
};

#endif /* DEEP_NESTED_STRUCTS_H */
