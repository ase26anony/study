/* complex-types.h - Header with deeply nested delimiter patterns for gengtype coverage */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists */
typedef int (*simple_fp)(int, double);
typedef void (*(*complex_fp)(int (*)(char), double))(float);
typedef int (*(*nested_fp)(int (*(*)(void))[5]))(void);

/* 2. Multi-dimensional and variable-length arrays */
typedef int matrix_t[10][(sizeof(int) > 4) ? 8 : 4];
typedef struct {
    int len;
    int arr[];  /* Flexible array member */
} flexible_array_t;

/* 3. Struct with function pointer array */
struct Operations {
    int (*ops[5])(int, int);
    void (*(*signal[3])(int, void (*)(int)))(int);
};

/* 4. Deeply nested typedef chain */
typedef struct Node Node;
struct Node {
    int (*(*get_callback)(Node *))[10];
    Node *(*next)(Node *, int);
    int data[((sizeof(Node*) * 8) / 2)];
};

/* 5. Union with nested initializer-capable structures */
union NestedContainer {
    struct {
        int (*fp)(int[2][3]);
        double matrix[2][(4 + 1)];
    } s;
    struct {
        void (*(*arr[2])(void))[5];
    } t;
};

/* 6. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_ARRAY(n) int (*(*fp_array##n[n])(int))[n]
#define CREATE_NESTED_TYPE(t) typedef t (*(*wrap_##t)(t (*)(t)))(t)

/* 7. Using the macros */
DECLARE_COMPLEX_ARRAY(3);
CREATE_NESTED_TYPE(int);

/* 8. Struct with bitfield and array combination */
struct MixedDecl {
    unsigned int flags : ((sizeof(int) * 8) - 1);
    int (*callbacks[((8) + 2)])(struct MixedDecl *, int);
    union {
        int i;
        void (*fp)(int (*)(void));
    } u;
};

/* 9. Function prototype with complex return type */
int (*(*get_handler_table(void))[10])(int, char *);

/* 10. Typedef for pointer to array of function pointers */
typedef int (*(*(*triple_indirect)[5])(float))[3];

#endif /* COMPLEX_TYPES_H */
