/* { dg-do compile } */
/* Complex type declarations with nested delimiters for gengtype coverage */

#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* 1. Function pointers with nested argument lists */
typedef int (*simple_fp)(int, char);
typedef void (*fp_with_fp_arg)(int (*)(char), double);
typedef int (*(*nested_fp_return)(void))(int, int);
typedef void (*(*signal_like)(int, void (*)(int)))(int);

/* 2. Multi-dimensional arrays with complex size expressions */
struct Matrix {
    int rows;
    int cols;
    int data[10][(sizeof(int) > 4) ? 8 : 16];
};

/* 3. Flexible array member in nested struct */
struct Container {
    int id;
    struct {
        int len;
        double items[];
    } flexible;
};

/* 4. Complex function pointer returning pointer to array */
typedef int (*(*callback_ret_array)(int param))[10];

/* 5. Struct with array of function pointers */
struct Operations {
    const char *name;
    int (*ops[5])(int (*)(char), double);
    void (*cleanup)(struct Operations *);
};

/* 6. Deeply nested parentheses in function pointer */
typedef int (*(*(*deep_nested)(int (*(*)(double))[3]))(void))[5];

/* 7. Union with anonymous struct containing array */
union Data {
    struct {
        int type;
        char buffer[(256 + sizeof(int))];
    };
    long long as_ll;
};

/* 8. Macro generating complex types */
#define DECLARE_FP_ARRAY(n, m) int (*(*fp_array##n##_##m[n])(int))[m]
#define NESTED_MACRO_TYPE(x) struct { int (*func)(int [(x)]); }

/* 9. Type with all three delimiters deeply nested */
typedef struct {
    int (*get_value)(int index, int (*validator)(int));
    char name[50];
    union {
        struct {
            int (*process)(char *data[(sizeof(char*) * 2)]);
            float matrix[3][(4 + 1)];
        };
        void (*fallback)(void);
    } strategies;
} ComplexType;

/* 10. Pointer to array of function pointers */
typedef int (*(*(*array_of_fp[3])(void))[2])(int, int);

/* 11. Self-referential structure with function pointer */
struct TreeNode {
    int value;
    struct TreeNode *(*get_child)(int index);
    struct TreeNode *children[4];
    void (*traverse)(struct TreeNode *, void (*visit)(int));
};

/* 12. Const volatile qualified function pointer */
typedef int (*(* const volatile cv_fp)(const int))[10];

/* 13. Nested struct with bitfield and array */
struct Packet {
    unsigned int header : 8;
    unsigned int flags : 8;
    struct {
        int seq;
        char data[256 - sizeof(int)];
    } payload;
    int (*checksum)(struct Packet *);
};

/* 14. Function returning function pointer */
int (*get_operation(int opcode))(int, int);

/* 15. Variable declaration with complex type */
extern int (*(*global_handler)(int (*)(char)))[(16 * sizeof(int))];

#endif /* COMPLEX_TYPES_H */
