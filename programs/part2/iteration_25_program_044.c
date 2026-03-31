/* Test file 1: Deeply nested structs and unions */
#ifndef DEEP_NESTED_STRUCTS_H
#define DEEP_NESTED_STRUCTS_H

/* Struct with deeply nested anonymous structs and unions */
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
                        int e:4;
                        int f:12;
                    } bits;
                } inner_union;
            } inner_struct;
        } u;
    } anonymous;
    
    /* Array with multiple dimensions */
    int multi_dim_array[3][4][5][6];
};

/* Union with nested struct containing bitfields */
union ComplexUnion {
    struct {
        unsigned int flag1:1;
        unsigned int flag2:2;
        unsigned int flag3:3;
        struct {
            unsigned int nested_flag1:4;
            unsigned int nested_flag2:4;
        } nested_bits;
    } bits;
    
    struct {
        char data[16];
        struct {
            int *ptr_array[8];
        } ptr_container;
    } data;
};

/* Struct with designated initializers in type definition context */
struct WithInitializer {
    int x;
    struct {
        int a;
        int b;
    } inner;
    int arr[2][3];
};

/* Function returning pointer to nested struct */
struct Level1 *(*get_complex_struct(void))[5];

/* Typedef with complex nested structure */
typedef struct {
    union {
        struct {
            int (*callback)(int, char);
        } s;
        void *generic;
    } u;
    int (*matrix[2][3])(float, double);
} UltraComplexType;

/* Macro expanding to nested type */
#define NESTED_PTR(type) type (*(*)[5])
NESTED_PTR(int) global_nested_ptr;

#endif /* DEEP_NESTED_STRUCTS_H */
