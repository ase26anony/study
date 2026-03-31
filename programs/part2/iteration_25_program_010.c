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
struct MatrixContainer {
    struct {
        int rows;
        int cols;
        union {
            float data[10][20];
            double ddata[5][5];
            struct {
                char* ptrs[3][4];
            } ptr_struct;
        } matrix_union;
    } config;
    
    /* Nested anonymous struct with bitfields */
    struct {
        unsigned int flag1 : 1;
        unsigned int flag2 : 3;
        struct {
            unsigned int nested_flag : 2;
            unsigned int : 4; /* Padding */
        } bitfield_struct;
    } flags;
};

/* Level 3: Deep nesting with all delimiters */
struct UltimateNest {
    /* Parentheses in function pointer */
    void (*callback)(int, char);
    
    /* Array of structs */
    struct {
        int id;
        union {
            /* Nested array in union */
            int arr[3][2];
            /* Pointer to array */
            int (*ptr_to_arr)[2];
        } data;
    } items[5];
    
    /* Struct containing union containing struct... */
    struct outer_most {
        union middle_union {
            struct inner_most {
                int deepest;
                /* Function returning pointer to function */
                int (*(*func_ptr)(void))(int);
            } inner;
            /* Array of function pointers */
            void (*callbacks[3])(struct inner_most*);
        } middle;
    } container;
};

/* Designated initializers with nested braces */
struct WithInitializer = {
    .a = 10,
    .nested = {
        .b = 'x',
        .u = {
            .inner_inner = {
                .e = 3.14f,
                .f = 2.71828
            }
        }
    }
};

#endif /* NESTED_STRUCTS_H */
