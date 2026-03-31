#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Parentheses: Function pointers with nested argument lists */
typedef int (*simple_fp)(int, double);
typedef void (*fp_with_fp_arg)(int (*)(char), double);
typedef int (*(*nested_fp_returning_fp)(void))(int, int);
typedef void (*(*signal_proto)(int, void (*)(int)))(int);

/* Brackets: Multi-dimensional and variable arrays */
typedef int matrix_2d[10][20];
typedef int vla_2d[][(sizeof(int) * 2)];
typedef int (*array_of_fp[5])(void);

/* Braces: Structs with nested initializers */
struct Point {
    int x;
    int y;
    int z;
};

struct NestedData {
    int id;
    struct Point points[3];
    int (*operations[2])(int, int);
};

/* Combined: Function pointer returning pointer to array */
typedef int (*(*callback_complex)(void))[10];

/* Struct containing array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*cleanup)(struct Operations *);
};

/* Macro with parentheses */
#define DECLARE_COMPLEX_ARRAY(n) int (*(*fp_array##n[n])(int))[n]
#define CREATE_NESTED_TYPE(t) typedef t (*nested_##t##_ptr)(t (*)(t), t)

/* Extern declarations for multi-file testing */
extern struct NestedData global_data;
extern int (*(*global_callback)(int))[3];
extern void register_operations(struct Operations *ops);

#endif /* COMPLEX_TYPES_H */
