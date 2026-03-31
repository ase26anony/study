/* { dg-do compile } */
/* Primary header with complex nested delimiter patterns */

#ifndef FILE1_H
#define FILE1_H

/* 1. Function pointers with nested argument lists */
typedef int (*fp_simple)(int, char);
typedef void (*(*fp_complex)(int (*)(char), double))(float);
typedef int (*(*signal_handler)(int sig, void (*)(int)))(int);

/* 2. Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(n) ((n) > 0 ? (n) : 1)

struct Matrix {
    int rows;
    int cols;
    int data[DYNAMIC_SIZE(10)][DYNAMIC_SIZE(20)];
};

/* 3. Combined: function pointer returning pointer to array */
typedef int (*(*Callback)(void))[10];
typedef void (*(*(*triple_indirect)(int))[5])(char);

/* 4. Struct with array of function pointers */
struct Operations {
    const char *name;
    int (*ops[5])(int, int);
    void (*(*advanced[3])(int))[10];
};

/* 5. Deeply nested parentheses in function declarations */
int (*(*(*create_factory(int level))(int (*)(int)))(int))[10];

/* 6. Flexible array member in nested struct */
struct Container {
    int id;
    struct {
        int count;
        int items[];
    } flexible;
};

/* 7. Union with nested initializer-style type */
union NestedUnion {
    struct {
        int (*func_ptr)(int, int);
        int matrix[3][(sizeof(int) + 2)];
    } s;
    long long data;
};

/* 8. Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_TYPE(n) \
    typedef int (*(*complex_fp##n)(int))[n]; \
    complex_fp##n var##n

DECLARE_COMPLEX_TYPE(5);
DECLARE_COMPLEX_TYPE(10);

/* 9. Type with all three delimiters deeply nested */
typedef struct {
    int (*(*get_callback)(int arg))[(arg > 0 ? arg : 1)];
    struct {
        int values[3][2];
        void (*initializer)(int[][2]);
    } config;
} MasterType;

/* 10. Variable declarations with nested delimiters */
extern int (*(*global_table)[(sizeof(int)*8)]);
extern void (*(*handlers[5]))(int);

#endif /* FILE1_H */
