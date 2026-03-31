/* complex-types.h - Test file for gengtype balanced delimiter parsing */
/* This file contains complex type declarations with nested parentheses, */
/* brackets, and braces to test consume_balanced() function coverage. */

#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists (parentheses) */
typedef int (*simple_fp)(int);
typedef void (*complex_fp1)(int (*)(char), double);
typedef int (*(*signal_proto)(int, void (*)(int)))(int);
typedef char *(*(**complex_fp2)(float, double(*)(int)))(long, short);

/* 2. Multi-dimensional arrays with nested size expressions (brackets) */
typedef int matrix_2d[10][20];
typedef int matrix_complex[(sizeof(int) > 4) ? 8 : 16][(1 << 3)];
extern int variable_matrix[][(10 + 5)];

/* 3. Structs with flexible array members and nested initializers (braces) */
struct nested_data {
    int id;
    struct {
        int x;
        int y;
        int z;
    } coord;
    union {
        int ival;
        float fval;
        char *pval;
    } data;
    int arr[];
};

/* 4. Combined: Struct containing array of function pointers */
struct operations {
    const char *name;
    int (*ops[5])(int, int);
    void (*init)(struct nested_data *);
    int (*process)(int (*)(int), int);
};

/* 5. Deeply nested typedef with all delimiter types */
typedef struct {
    int (*(*get_callback)(void))[10];
    struct operations *(*find_op)(const char *);
    int matrix[3][(sizeof(double) + 2)];
} container_t;

/* 6. Union with anonymous struct containing function pointer array */
union mixed_types {
    struct {
        int (*comparators[3])(const void *, const void *);
        void (*loggers[2])(const char *, ...);
    } funcs;
    struct nested_data *data;
    container_t *container;
};

/* 7. Macro generating complex types with parentheses */
#define DECLARE_CALLBACK(n) \
    typedef int (*(*callback_##n)(int (*(*)(void))[n]))(void)

DECLARE_CALLBACK(5);
DECLARE_CALLBACK(10);

/* 8. Function prototype with complex return type */
int (*(*register_handler(const char *name, 
                         void (*callback)(int, const char *)))
     (int, void *))[5];

/* 9. Nested struct with bitfields and array */
struct bitfield_container {
    unsigned int flags : 4;
    unsigned int : 4;  /* Padding */
    struct {
        int count;
        char *items[8];
    } section;
    int (*validate)(struct bitfield_container *, int);
};

/* 10. Typedef for extremely complex function pointer */
typedef void (*(*(*ultimate_fp)(
    int, 
    char *(*)(int, const char **), 
    struct operations *
))(double, float (*)(int)))(long);

#endif /* COMPLEX_TYPES_H */
