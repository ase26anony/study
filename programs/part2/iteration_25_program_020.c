/* Test file 1: Deeply nested structs and unions */
#ifndef DEEP_NESTED_STRUCTS_H
#define DEEP_NESTED_STRUCTS_H

/* Struct with deeply nested anonymous structs and unions */
struct Level1 {
    struct {
        union {
            struct {
                int a;
                struct {
                    char b;
                    union {
                        short c;
                        long d;
                        struct {
                            float e;
                            double f;
                        } inner_most;
                    } deep_union;
                } level4;
            } level3;
            long long alternate;
        } level2;
    } level1_anon;
    
    /* Array with multiple dimensions */
    int multi_dim_array[3][4][5][6];
    
    /* Pointer to array of pointers to structs */
    struct Level1 *(*ptr_array[10])[5];
};

/* Union with nested struct containing bit-fields */
union ComplexUnion {
    struct {
        unsigned int flag1 : 1;
        unsigned int flag2 : 3;
        unsigned int : 4;  /* Padding */
        unsigned int value : 24;
        struct {
            char a;
            int b;
        } nested_in_bitfield;
    } bitfield_struct;
    
    /* Array in union */
    double matrix[2][2];
    
    /* Pointer to function returning pointer to struct */
    struct Level1 *(*get_level1)(void);
};

/* Struct with designated initializers in type definition */
struct WithDesignatedInit {
    int x;
    struct {
        int a;
        int b;
    } point;
    int arr[5];
} global_var = {
    .x = 42,
    .point = { .a = 1, .b = 2 },
    .arr = { [0] = 10, [2] = 20, [4] = 30 }
};

/* Multiple levels of nesting in a single declaration */
struct UltimateNest {
    struct {
        union {
            struct {
                int (*(*complex_func_ptr)(struct UltimateNest *))[10];
                void (*callback)(int, char, struct {
                    double x;
                    double y;
                });
            } func_nest;
            char simple;
        } choice;
        int array_of_ptrs[5][7];
    } container;
};

#endif /* DEEP_NESTED_STRUCTS_H */
