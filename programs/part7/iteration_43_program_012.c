/* complex-types.h - Header with deeply nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists */
typedef int (*simple_fp)(int, char);
typedef int (*complex_fp)(int (*)(char), double);
typedef void (*(*signal_proto)(int, void (*)(int)))(int);

/* 2. Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 0 ? (x) : 1)

struct Matrix {
    int rows;
    int cols;
    int data[DYNAMIC_SIZE(10)][DYNAMIC_SIZE(5)];
};

/* 3. Nested struct with flexible array member */
struct Container {
    int id;
    struct {
        int count;
        int items[];
    } nested;
};

/* 4. Function returning pointer to array */
typedef int (*(*array_returner)(void))[10];

/* 5. Deeply nested function pointer type */
typedef int (*(*(*deep_nested_fp)(int (*(*)(char))[5]))(double))[3];

/* 6. Struct with array of function pointers */
struct Operations {
    const char *name;
    int (*ops[5])(int, int);
    void (*cleanup)(struct Operations *);
};

/* 7. Union with nested initializer-style type */
union Variant {
    int (*as_func)(int);
    int (*as_array)[(sizeof(int) * 2)];
    struct {
        int tag;
        union Variant *next;
    } linked;
};

/* 8. Macro generating complex types */
#define DECLARE_COMPLEX(n) \
    typedef int (*(*complex_type_##n)(int))[n]; \
    complex_type_##n create_complex_##n(void)

DECLARE_COMPLEX(5);
DECLARE_COMPLEX(10);

/* 9. Type with all three delimiters deeply nested */
typedef struct {
    int (*processor)(int matrix[][(10)], void (*callback)(int));
    union {
        int values[5];
        struct {
            int x, y;
        } point;
    } data;
} MasterType;

/* 10. Function prototypes using complex types */
extern void process_matrix(int (*matrix)[][(sizeof(int)*8)], int size);
extern struct Operations *create_ops(int (*init)(int, int));
extern int (*(*get_array_func(void))[5])(int, int);

#endif /* COMPLEX_TYPES_H */
