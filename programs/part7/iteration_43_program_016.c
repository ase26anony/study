/* complex-types.h - Header with deeply nested delimiter patterns for gengtype coverage */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*simple_fp)(int, char);
typedef int (*complex_fp)(int (*)(char), double);
typedef void (*(*signal_proto)(int sig, void (*func)(int)))(int);

/* Test 2: Multi-dimensional arrays with complex size expressions */
#define DYNAMIC_SIZE(x) ((x) > 10 ? 20 : 5)
extern int multi_array[10][DYNAMIC_SIZE(15)][(sizeof(int)*2)];

/* Test 3: Struct with flexible array member and nested struct */
struct outer_struct {
    int id;
    struct {
        int x;
        int y;
        int (*calc)(struct outer_struct *, int);
    } inner;
    int data[];
};

/* Test 4: Union with complex nested types */
union complex_union {
    int (*fp_array[5])(int, int);
    struct {
        int count;
        int (*matrix[3][3])(void);
    } nested;
    long long big_val;
};

/* Test 5: Typedef combining function pointer returning array pointer */
typedef int (*(*callback_proto)(void))[10];
typedef void (*(*(*triple_indirect)(int (*(*)(double))[5]))(char))[3];

/* Test 6: Struct containing array of function pointers */
struct operations {
    const char *name;
    int (*ops[7])(int, int, int (*)(int));
    struct operations *next;
};

/* Test 7: Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_FP(n) int (*(*fp##n)(int (*(*)(double))[n]))[n]
DECLARE_COMPLEX_FP(5);

/* Test 8: Nested anonymous struct/unions */
struct container {
    int type;
    union {
        struct {
            int (*handler)(struct container *, int);
            int data[4][4];
        } s;
        struct {
            void (*cleanup)(void **);
            int *ptr_arr[(sizeof(void*) * 8)];
        } u;
    } variant;
};

/* Test 9: Function prototype with deeply nested parameters */
extern int process_matrix(int (*matrix)[][(sizeof(int) + 2)], 
                         int (*(*get_func)(int))[5],
                         void (*callback)(int, ...));

/* Test 10: Type with all three delimiters deeply nested */
typedef struct {
    int (*initialize)(int, char *(*)(void));
    struct {
        int values[3][(16/sizeof(int))];
        void (*methods[2])(int (*)(int), int[]);
    } config;
    union {
        long (*transform)(int (*)(long), long[]);
        double matrix[2][(sizeof(double) > 4 ? 3 : 6)];
    } alt;
} ultimate_type_t;

#endif /* COMPLEX_TYPES_H */
