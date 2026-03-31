/* Test input for gengtype parser coverage */
/* This file is processed by gengtype during GCC build */

/* GTY marker example */
typedef struct GTY(()) TreeNode {
    struct TreeNode *left;
    struct TreeNode *right;
    int value;
} TreeNode;

/* Parentheses case: function pointer declarations */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_fp)(void))(int);
int (*callbacks[5])(const char*);

/* Brackets case: array declarations */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
char* string_table[3][20];

/* Braces case: struct/enum definitions and initializers */
struct Data {
    int vals[2];
    char name[32];
};

union Container {
    int i;
    float f;
    struct {
        int x;
        int y;
    } point;
};

enum Color { RED, GREEN, BLUE };

/* Combined cases: nested delimiters */
int (*(*get_array_ptr(void))[10]);
struct Nested {
    int (*func_array[3])(void);
    struct {
        int matrix[2][2];
    } inner;
};

/* GTY-marked variable with complex type */
static GTY(()) int (*global_hook)(int) = NULL;

/* Array initializer with braces */
int global_vec[3] = {1, 2, 3};

/* Struct with nested initializer */
struct Data d = { .vals = {10, 20}, .name = "test" };

/* Function pointer with nested parentheses */
typedef void (*(*(*deep_nested)(int))[5])(char);

/* Multi-dimensional array with size expression */
int dynamic_like[sizeof(int) > 4 ? 8 : 4][(2+3)*2];
