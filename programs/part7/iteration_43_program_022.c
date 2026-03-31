/* complex-types.h - Header with deeply nested delimiter patterns for gengtype coverage */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists */
typedef int (*simple_fp)(int, double);
typedef void (*(*complex_fp)(int (*)(char), double))(float);
typedef int (*(*nested_fp_arg)(int (*(*)(int[5]))(void)))(char);

/* 2. Multi-dimensional and variable-length arrays */
typedef int matrix_t[10][(sizeof(int) > 4) ? 8 : 4];
typedef char vla_t[][(16 * 2)];

/* 3. Struct with flexible array member and nested function pointers */
struct container {
    int (*ops[3])(int, int);
    struct inner {
        int (*nested_fp)(struct container *);
        int arr[2][3];
    } inner;
    int flex[];
};

/* 4. Union with complex nested types */
union polytype {
    int (*fp)(int);
    struct {
        int (*matrix_ptr)[10][20];
        union {
            int val;
            void (*action)(void);
        } u;
    } s;
};

/* 5. Deeply nested typedef chain */
typedef int (*(*level1)(void))[5];
typedef level1 (*(*level2)(level1))[10];
typedef level2 (*(*level3)(level2, level1))(void);

/* 6. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_ARRAY(n) int (*(*fp_array##n[n])(int))[n]
#define NESTED_MACRO_TYPE(x) struct { int (*func)(int arr[x][x]); }

/* 7. Struct with bitfields and arrays */
struct bitfield_struct {
    unsigned int flags : 3;
    int (*callbacks[4])(struct bitfield_struct *);
    int matrix[2][(8 + 2)];
};

/* 8. Typedef with all three delimiters combined */
typedef struct {
    int (*(*get_matrix)(void))[10][20];
    void (*init)(int (*)(int[5]), ...);
    struct node *children[];
} combined_t;

/* 9. Function pointer returning pointer to array of function pointers */
typedef int (*(*(*meta_fp)(void))[5])(int, int);

/* 10. Struct with anonymous union/struct */
struct anonymous_example {
    union {
        int (*fp)(int);
        struct {
            int count;
            int (*methods[2])(void);
        } s;
    } u;
    int data;
};

#endif /* COMPLEX_TYPES_H */
