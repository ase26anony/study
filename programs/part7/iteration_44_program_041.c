/* Test file to exercise gengtype's balanced delimiter parsing */

/* Parentheses cases - function pointers */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_fp)(void))(int);
int (*callback)(const char*, ...);

/* Brackets cases - arrays */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
char* string_table[][3];

/* Braces cases - struct/enum definitions and initializers */
struct SimpleStruct {
    int field1;
    char field2;
};

enum Color { RED, GREEN, BLUE };

static int initialized_array[3] = {1, 2, 3};

/* Nested/combined cases */
int (*array_of_funcs[5])(const char*);  /* [] + () */
int (*(*get_matrix_ptr(void))[10][20]);  /* () + * + [] */

struct Container {
    int (*compare)(const void*, const void*);  /* struct + () */
    int data[(sizeof(int) * 4)];               /* struct + [] */
};

union Variant {
    int i;
    float f;
    struct {
        int x;
        int y;
    } point;                                   /* union + struct + {} */
};

/* GTY-marked declarations (common in GCC sources) */
typedef struct GTY(()) TreeNode {
    struct TreeNode *GTY((skip)) left;
    struct TreeNode *right;
    int value;
} TreeNode;

static GTY(()) int (*global_handler)(int) = NULL;

/* Complex nested example */
typedef void (*(*signal_handler)(int, void(*)(int)))(void);
