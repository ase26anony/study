/* complex_types.h - Header with complex type declarations for gengtype testing */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested argument lists */
typedef int (*simple_fp)(int, char);
typedef int (*complex_fp)(int (*)(char), double);
typedef void (*(*signal_fp)(int sig, void (*func)(int)))(int);

/* Test 2: Multi-dimensional arrays with nested size expressions */
#define DYNAMIC_SIZE(x) ((x) > 0 ? (x) : 1)

struct ArrayStruct {
    int matrix[10][DYNAMIC_SIZE(5)];
    int (*vla)[DYNAMIC_SIZE(3)];
};

/* Test 3: Flexible array member in nested struct */
struct Outer {
    int id;
    struct {
        int len;
        int data[];
    } inner;
};

/* Test 4: Function pointer returning pointer to array */
typedef int (*(*Callback)(void))[10];

/* Test 5: Struct containing array of function pointers */
struct Operations {
    int (*ops[5])(int, int);
    void (*(*advanced[3])(int))[2];
};

/* Test 6: Macro expanding to complex type with parentheses */
#define DECLARE_COMPLEX(n) int (*(*fp##n)(int))[n]

/* Test 7: Deeply nested parentheses in function type */
typedef int (*(*(*deep_nested)(int (*(*)(double))[3]))(char))[5];

/* Test 8: Union with nested struct initializer pattern */
union NestedUnion {
    struct {
        int x;
        int y[2][2];
    } point;
    struct {
        void (*func)(int);
        int arr[][3];
    } func_data;
};

/* Test 9: Type with all three delimiters deeply nested */
typedef struct {
    int (*(*get_matrix)(int size))[][(sizeof(int)*8)];
    void (*init)(struct ArrayStruct (*)[DYNAMIC_SIZE(2)]);
} Container;

/* Test 10: Variable declarations with casts in array bounds */
extern int (*global_table)[(sizeof(int)*8)];
extern void (*(*global_handler)[(2+3)])(int);

/* Function prototypes using complex types */
Callback create_callback(void);
int execute_operation(struct Operations *ops, int idx, int a, int b);
void process_container(Container *c, int size);

#endif /* COMPLEX_TYPES_H */
