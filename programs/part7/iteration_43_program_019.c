/* complex_types.h - Header with deeply nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*simple_fp)(int, int);
typedef int (*complex_fp)(int (*)(char), double);
typedef void (*(*signal_proto)(int sig, void (*func)(int)))(int);

/* Test 2: Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 0 ? (x) : 1)

struct ArrayStruct {
    int matrix[10][DYNAMIC_SIZE(5)];
    int (*vla)[DYNAMIC_SIZE(3)];
};

/* Test 3: Nested struct with flexible array member */
struct NestedContainer {
    struct {
        int len;
        int arr[];
    } inner;
    struct ArrayStruct *ptr_array[5];
};

/* Test 4: Function returning pointer to array */
typedef int (*(*Callback)(void))[10];

/* Test 5: Struct containing array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*(*complex_ops[3])(int))[2];
};

/* Test 6: Macro generating complex types with parentheses */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]
DECLARE_COMPLEX(5);
DECLARE_COMPLEX(10);

/* Test 7: Union with nested struct initializer syntax */
union ComplexUnion {
    struct {
        int x;
        int y[2][3];
    } point;
    void (*func_ptr)(int, ...);
};

/* Test 8: Type with all three delimiters deeply nested */
typedef struct {
    int (*(*get_matrix)(int size))[][];
    void (*init)(struct NestedContainer (*containers)[3]);
} MasterType;

/* Test 9: Variable declarations with nested sizeof */
extern int (*global_table)[(sizeof(int)*8)];
extern char *(*string_ptrs)[(sizeof(struct ArrayStruct) + 16)];

/* Test 10: Function prototypes with complex parameter types */
void process_matrix(int (*matrix)[][(10+5)], int rows);
struct Operations *create_ops(int (*(*factory)(int))[]);

#endif /* COMPLEX_TYPES_H */
