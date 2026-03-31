/* Test input for gengtype parser coverage */
/* This file contains constructs that will trigger consume_balanced() calls */

/* GTY marker for gengtype recognition */
#define GTY(x) __attribute__((gty))

/* 1. Parentheses () - Function pointer declarations */
typedef int (*simple_func_ptr)(int, char);
typedef void (*(*complex_func_ptr)(void))(int);
GTY(()) int (*global_func)(double);

/* 2. Brackets [] - Array declarations */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
GTY(()) char *string_array[(10 + 5)];

/* 3. Braces {} - Aggregate definitions */
struct SimpleStruct {
    int field1;
    char field2;
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

/* 4. Nested combinations - Exercise recursive consume_balanced */

/* Array of function pointers ([] and ()) */
int (*callback_array[5])(const char*);

/* Function pointer returning pointer to array ((), *, and []) */
int (*(*get_matrix_ptr(void))[10]);

/* Struct with initialized array member ({} and []) */
struct DataContainer {
    int values[2];
    char *names[3];
};

/* 5. Complex nested examples */

/* Multi-level nesting */
typedef struct TreeNode {
    struct TreeNode *left;
    struct TreeNode *right;
    void (*print_func)(struct TreeNode *);
    int data[(2 * sizeof(void*))];
} TreeNode;

/* Function with array parameter and function pointer return */
double (*transform_matrix(int rows, int cols, double matrix[rows][cols]))(int);

/* Union with anonymous struct containing array */
union Container {
    struct {
        int ids[4];
        char tags[2][10];
    } items;
    long raw_data[8];
};

/* 6. Initializers with braces */
GTY(()) int initialized_array[3] = {1, 2, 3};
struct Point {
    int x;
    int y;
} points[2] = {{1, 2}, {3, 4}};

/* 7. More edge cases */

/* Pointer to array of function pointers */
int (*(*func_table[3])[5])(void);

/* Nested parentheses in sizeof expressions */
char buffer[sizeof(struct { int a; double b; })];

/* Struct with bitfield (still uses braces) */
struct BitfieldStruct {
    unsigned int flag:1;
    unsigned int value:7;
};

/* 8. Declaration with all three delimiters */
struct CompleteExample {
    int (*methods[2])(struct CompleteExample *);
    union {
        int num;
        char str[20];
    } data;
} example = {
    .methods = {NULL, NULL},
    .data = {.str = "test"}
};

/* End of test input */
