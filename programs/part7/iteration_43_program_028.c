/* complex-types.h - Header with deeply nested delimiter patterns */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test 1: Function pointers with nested parentheses */
typedef int (*simple_fp)(int, int);
typedef void (*(*complex_fp)(int (*)(char), double))(float);
typedef int (*(*signal_proto)(int, void (*)(int)))(int);

/* Test 2: Multi-dimensional arrays with nested brackets */
typedef int matrix_2d[10][20];
typedef int (*array_of_fp[5])(int, int);
typedef int (*(*nested_array_fp)(void))[10];

/* Test 3: Structs with nested braces in declarations */
struct Point {
    int x;
    int y;
    int z;
};

struct NestedStruct {
    struct Point points[3];
    int (*operations[2])(struct Point, struct Point);
    union {
        int i;
        float f;
        struct {
            char c;
            short s;
        } inner;
    } data;
};

/* Test 4: Combined patterns - struct with function pointer returning array pointer */
struct Container {
    int (*(*get_matrix)(int size))[][10];
    void (*processor)(int (*callback)(int, int), int);
    struct NestedStruct items[(sizeof(int) * 2)];
};

/* Test 5: Deeply nested parentheses for function pointers */
typedef int (*(*(*deep_nested_fp)(int (*(*)(double))[3]))(char))[5];

/* Test 6: Variable-length array in struct (flexible array member) */
struct VLAContainer {
    int count;
    int data[];  /* Flexible array member */
};

/* Test 7: Function returning pointer to function with array parameter */
typedef float (*(*func_ret_func)(int arr[][(2+3)]))(double);

/* Test 8: Macro generating complex types with parentheses */
#define DECLARE_COMPLEX_ARRAY(n) int (*(*fp_array##n)[n])(int, int)
#define CREATE_NESTED_TYPE(t) typedef t (*(*nested_##t##_ptr)(void))[]

/* Use the macros */
DECLARE_COMPLEX_ARRAY(3);
DECLARE_COMPLEX_ARRAY(5);

/* Test 9: Anonymous struct with bitfields and nested unions */
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    union {
        struct {
            int a : 4;
            int b : 4;
        } parts;
        char whole;
    } value;
    int (*validate)(struct BitFieldStruct *);
};

/* Test 10: Typedef chain with increasing complexity */
typedef int basic_t;
typedef basic_t (*basic_fp_t)(basic_t);
typedef basic_fp_t (*complex_fp_t)(basic_fp_t, basic_t (*)(basic_t));
typedef complex_fp_t (*very_complex_fp_t)(complex_fp_t, basic_fp_t (*)(complex_fp_t));

/* Forward declarations for mutual recursion */
struct TreeNode;
typedef struct TreeNode (*tree_visitor)(struct TreeNode *, void (*)(struct TreeNode *));

struct TreeNode {
    int value;
    struct TreeNode *left;
    struct TreeNode *right;
    tree_visitor visit;
};

/* Test 11: Array of pointers to functions returning pointers to arrays */
typedef int (*(*array_func_ptr[3])(void))[10];

/* Test 12: Const-volatile qualified function pointers */
typedef int (*(* const volatile cv_fp)(const int, volatile char))(void);

/* Test 13: Nested sizeof expressions in array bounds */
extern int sized_array[sizeof(struct Point) + sizeof(int(*[2])(int, int))];

/* Test 14: Alignment-specific types */
typedef struct {
    long double ld __attribute__((aligned(16)));
    int (*align_check)(void * __attribute__((aligned(8))));
} AlignedStruct;

#endif /* COMPLEX_TYPES_H */
