/* Test file to exercise gengtype's balanced delimiter parsing */

/* Parentheses in function pointer declarations */
typedef int (*func_ptr_type)(int, char);
void (*(*complex_func_ptr)(void))(int);

/* Brackets in array declarations */
int simple_array[10];
extern int multi_dim_array[5][(sizeof(int)*2)];

/* Braces in aggregate definitions */
struct SimpleStruct {
    int field1;
    char field2;
};

union TestUnion {
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

/* Nested and combined delimiters */

/* Array of function pointers - combines [] and () */
int (*callback_array[5])(const char*);

/* Function pointer returning pointer to array - combines (), *, and [] */
int (*(*get_array_pointer(void))[10]);

/* Struct with array member and initializer - combines {} and [] */
struct DataContainer {
    int values[2];
    char *names[3];
};

/* GTY-marked declarations (if GTY is recognized) */
typedef struct GTY(()) TreeNode {
    struct TreeNode *GTY((skip)) left;
    struct TreeNode *right;
    int value;
} TreeNode;

/* Static initializer with braces */
static int initialized_array[3] = {1, 2, 3};

/* Complex nested example */
struct Outer {
    int (*comparator)(const void*, const void*);
    struct {
        int (*handlers[4])(void);
        char buffer[256];
    } inner;
};

/* Function pointer with array parameter */
void (*signal_handler)(int sig, void *context[2]);

/* Typedef with all three delimiters */
typedef struct {
    int (*methods[3])(int, char*);
    union {
        int i;
        float f;
    } data;
} Object;
