/* test-gengtype-input.c - Input to trigger coverage of balanced delimiter parsing */

/* 1. Parentheses () cases */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_func_ptr)(void))(int);
int (*signal(int sig, void (*handler)(int)))(int);

/* 2. Brackets [] cases */
int array[10];
extern int matrix[5][(sizeof(int)*2)];
char *string_array[] = {"hello", "world"};

/* 3. Braces {} cases */
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

int global_vec[3] = {1, 2, 3};

/* 4. Nested and combined cases */
/* Array of function pointers - combines [] and () */
int (*callbacks[5])(const char*);

/* Function pointer returning pointer to array - combines (), *, and [] */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member initialized in-line - combines {} and [] */
struct Data {
    int vals[2];
    char *names[3];
} data_instance = { 
    .vals = {10, 20},
    .names = {"a", "b", "c"}
};

/* 5. In GTY contexts (if GTY is recognized) */
/* GTY marker with function pointer */
typedef struct GTY(()) TreeNode {
    struct TreeNode *left;
    struct TreeNode *right;
    void (*visit)(struct TreeNode *);
} TreeNode;

/* GTY marker with array */
static GTY(()) int (*global_hooks[4])(int) = {NULL, NULL, NULL, NULL};

/* 6. Complex nested example */
typedef void (*(*(*nested_fp)(int))[5])(char);

/* 7. Multiple levels of nesting */
struct Container {
    int (*operations[3])(struct Container *);
    union {
        int (*int_func)(int);
        void (*void_func)(void);
    } func_union;
    struct {
        int matrix[2][(2+3)];
    } data_block;
};

/* 8. Edge cases with empty delimiters */
typedef void (*empty_args_func)(void);
int empty_array[] = {};
struct EmptyStruct {};
