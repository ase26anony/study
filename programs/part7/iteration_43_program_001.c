/* complex-types.h - Primary header with deeply nested delimiter patterns */

#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Level 1: Basic nested parentheses for function pointers */
typedef int (*simple_fp)(int, char);
typedef void (*(*signal_proto)(int, void (*)(int)))(int);

/* Level 2: Function pointer with nested function pointer argument */
typedef int (*complex_fp1)(int (*)(char, double), void*);
typedef void (*(*complex_fp2)(int (*(*)(int[5]))(void)))(double);

/* Level 3: Multi-dimensional array types with nested size expressions */
typedef int matrix_t[10][(sizeof(int) > 4) ? 8 : 4];
typedef char buffer_t[(1 << 3) + (sizeof(long) * 2)];

/* Level 4: Struct with flexible array member and nested bitfield */
struct nested_container {
    int tag;
    union {
        struct {
            int (*handler)(struct nested_container*, int);
            int data[((sizeof(int) * 8) - 1)];
        } s;
        void (*alt_handler)(void (*)(int), int);
    } u;
    int flexible_arr[];
};

/* Level 5: Deeply nested function pointer returning array pointer */
typedef int (*(*(*deep_nested_fp)(int (*(*)(int[3]))(void)))[10])(char);

/* Level 6: Struct containing array of function pointers */
struct operations {
    int count;
    int (*(*func_table[5])(int, int (*(*)(char))(void)))(double);
    struct operations* (*(*self_ref)(struct operations*))[2];
};

/* Level 7: Union with anonymous struct containing nested arrays */
union variant {
    struct {
        int (*compare)(union variant*, union variant*);
        int values[2][3][((2 + 3) * sizeof(int))];
    } s;
    void (*action)(int, ...);
    long (*(*array_maker)(int))[];
};

/* Level 8: Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_TYPE(n) \
    typedef int (*(*fp_type##n)(int (*(*)[n])(void)))[(n) * 2]

DECLARE_COMPLEX_TYPE(1);
DECLARE_COMPLEX_TYPE(2);
DECLARE_COMPLEX_TYPE(3);

/* Level 9: Type with all three delimiters deeply nested */
typedef struct {
    int (*init)(struct {
        int x;
        int y[((sizeof(double) + 3) & ~3)];
    }*);
    void (*(*cleanup[2])(int, ...))(void);
    union {
        int (*int_op)(int, int);
        void (*(*void_op)(void))[(1 << 4)];
    } ops;
} master_type_t;

/* Level 10: Recursive type definition with nested delimiters */
typedef struct tree_node {
    struct tree_node* (*(*children)[3])(void);
    int (*(*value)(struct tree_node*))[(sizeof(int) + 7) / 8];
    void (*visit)(struct tree_node*, 
                  void (*callback)(int, 
                                   struct tree_node* (*(*)(int))[2]));
} tree_node_t;

#endif /* COMPLEX_TYPES_H */
