/* complex-types.h - Header with complex nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists */
typedef int (*simple_fp)(int, int);
typedef void (*complex_fp1)(int (*)(char), double);
typedef int (*(*complex_fp2)(int, void (*)(int)))(double);
typedef void (*(*signal_proto)(int, void (*)(int)))(int);

/* 2. Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 0 ? (x) : 1)
extern int multi_dim_array[10][DYNAMIC_SIZE(5)][(sizeof(int) * 2)];

/* 3. Struct with flexible array member and nested struct */
struct OuterStruct {
    int id;
    struct {
        int x;
        int y;
        int (*calc)(struct OuterStruct *, int);
    } inner;
    int data[];
};

/* 4. Union with nested struct and function pointer */
union ComplexUnion {
    struct {
        int type;
        union {
            int ival;
            double dval;
            void *pval;
        } value;
    } tagged;
    void (*processor)(union ComplexUnion *);
};

/* 5. Typedef combining function pointer returning array pointer */
typedef int (*(*Callback)(void))[10];
typedef void (*(*(*nested_fp)(int (*)(int)))[5])(double);

/* 6. Struct containing array of function pointers */
struct Operations {
    const char *name;
    int (*ops[5])(int, int);
    void (*(*advanced[3])(struct Operations *))[2];
};

/* 7. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]
DECLARE_COMPLEX(5);
DECLARE_COMPLEX(10);

/* 8. Nested typedef chain */
typedef struct Node Node;
struct Node {
    Node *next;
    Node *prev;
    void *data;
    int (*compare)(Node *, Node *);
};

/* 9. Function prototype with complex parameter */
extern void register_callback(int (*cb)(int, int (*)(char)), void *ctx);
extern int (*(*get_factory(const char *name))(int))[3];

/* 10. Anonymous struct in union with nested arrays */
union Container {
    struct {
        int matrix[2][(4 + 1)];
        char *strings[3];
    } s;
    struct {
        void *pointers[4];
        int (*funcs[2])(void);
    } u;
};

#endif /* COMPLEX_TYPES_H */
