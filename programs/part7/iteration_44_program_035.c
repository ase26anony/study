/* Test input for gengtype parser coverage */
/* This file contains constructs that trigger consume_balanced() calls */

/* GTY markers to ensure gengtype pays attention */
#define GTY(x) __attribute__((gty x))

/* 1. Parentheses () - Function pointer declarations */
typedef int (*func_ptr_type)(int, char);
GTY(()) int (*global_func_ptr)(void);
static void (*(*complex_func_ptr)(int))(char);

/* 2. Brackets [] - Array declarations */
GTY(()) int simple_array[10];
extern double matrix[5][(sizeof(double) * 2)];
typedef char string_array[][32];

/* 3. Braces {} - Struct/union/enum definitions and initializers */
struct SimpleStruct {
    int field1;
    char field2;
};

GTY(()) struct TaggedStruct {
    struct SimpleStruct *next;
    int values[3];
};

union MixedUnion {
    int i;
    float f;
    struct {
        int x;
        int y;
    } point;
};

enum Color {
    RED,
    GREEN,
    BLUE
};

/* 4. Nested combinations */
/* Array of function pointers - combines [] and () */
int (*callback_array[5])(const char *);

/* Function pointer returning pointer to array - combines (), *, and [] */
int (*(*get_matrix_ptr(void))[10][20]);

/* Struct with initialized array member - combines {} and [] */
GTY(()) struct DataContainer {
    int measurements[4];
    float (*processor)(int);
} data_instance = { 
    .measurements = {1, 2, 3, 4},
    .processor = NULL
};

/* 5. Complex nested example */
typedef struct TreeNode {
    struct TreeNode *left;
    struct TreeNode *right;
    void (*visit)(struct TreeNode *);
    int keys[(16 + sizeof(void *))];
} TreeNode;

/* 6. More edge cases */
/* Pointer to array of function pointers */
int (*(*func_array_ptr)[8])(int);

/* Struct containing anonymous union with array */
struct WithAnonymous {
    union {
        int as_int[2];
        float as_float[2];
    };
    void (*handler)(int[]);
};

/* Initializer with nested braces */
static GTY(()) struct NestedInit {
    int a;
    struct {
        int b[2];
        int c;
    } inner;
} nested = {1, {{2, 3}, 4}};

/* Function type with array parameter (size in parentheses) */
typedef int array_func(int arr[(10 * sizeof(int))]);

/* Multi-dimensional array with computed sizes */
extern int dynamic_grid[][(sizeof(int) > 4 ? 8 : 16)];
