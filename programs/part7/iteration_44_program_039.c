/* Test input for gengtype parser coverage - targeting balanced delimiter parsing */

/* 1. Parentheses () cases - function pointers */
typedef int (*simple_func_ptr)(int, char);
typedef void (*(*complex_fp)(void))(int);
typedef int (*array_func_ptr[5])(void);

/* With GTY marker */
GTY(()) int (*global_callback)(const char *);

/* 2. Brackets [] cases - arrays */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
static char buffer[(1 << 8)];

/* Array with GTY marker */
GTY(()) struct Node *node_array[100];

/* 3. Braces {} cases - struct/enum definitions and initializers */
struct SimpleStruct {
    int a;
    char b;
};

union DataUnion {
    int i;
    float f;
    struct {
        int x;
        int y;
    } point;
};

enum Color { RED, GREEN, BLUE };

/* Static initializer with braces */
int global_init[3] = {1, 2, 3};
struct SimpleStruct s = { .a = 42, .b = 'X' };

/* 4. Nested combinations - exercise consume_balanced recursion */

/* Array of function pointers ([] and ()) */
int (*callbacks[5])(const char*);

/* Function pointer returning pointer to array ((), *, and []) */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member initialized inline ({}, [], and {}) */
struct Container {
    int values[2];
    void (*handler)(int);
} container = { 
    .values = {10, 20}, 
    .handler = NULL 
};

/* Complex nested example */
typedef struct TreeNode {
    struct TreeNode *children[(MAX_CHILDREN)];
    void (*visit)(struct TreeNode *);
    union {
        int int_val;
        float float_val;
    } data;
} TreeNode;

/* GTY-marked complex type */
GTY((chain_next = "next"))
struct GtyList {
    struct GtyList *next;
    void (*process)(struct GtyList *);
    int items[];
};

/* Multi-level nesting */
int (*(*(*nested_fp)(int))[5])(void);

/* Initializer with all delimiter types */
struct CompleteExample {
    int (*func)(int);
    int array[3];
    struct {
        char *name;
    } inner;
} example = {
    .func = NULL,
    .array = {1, 2, 3},
    .inner = { .name = "test" }
};

/* Function-like macro with parentheses (should be skipped) */
#define MAX(a,b) ((a) > (b) ? (a) : (b))

/* Empty braces/brackets/parentheses cases */
struct EmptyStruct {};
int empty_array[] = {};
void (*empty_func_ptr)(void);
