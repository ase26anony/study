/* test_gengtype_coverage.c
 * This file contains constructs specifically designed to exercise
 * the balanced delimiter parsing logic in gengtype-parse.cc.
 * It should be processed by gengtype with: ./gengtype -p test_gengtype_coverage.c /dev/null
 */

/* Parentheses cases - function pointers */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_fp)(void))(int);
int (*callback)(const char *msg, int len);

/* Brackets cases - arrays */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
char *string_table[20];

/* Braces cases - struct/enum definitions and initializers */
struct SimpleStruct {
    int a;
    char b;
};

enum Color { RED, GREEN, BLUE };

static int global_vec[3] = {1, 2, 3};

/* Nested combinations */

/* Array of function pointers - combines [] and () */
int (*callbacks[5])(const char*);

/* Function pointer returning pointer to array - combines (), *, and [] */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member initialized in-line - combines {} and [] */
struct Data {
    int vals[2];
} data_instance = { .vals = {10, 20} };

/* Complex nested example */
struct Container {
    int (*processor)(int (*)(int), int);
    struct {
        int matrix[3][4];
    } inner;
} container_var = {
    .processor = NULL,
    .inner = { .matrix = {{0}} }
};

/* GTY-marked declarations (if gengtype recognizes GTY) */
typedef struct GTY(()) GtyNode {
    struct GtyNode *GTY((skip)) next;
    int (*GTY((tag("FUNC_PTR"))) handler)(void);
} GtyNode;

/* Union with nested struct */
union ComplexUnion {
    int i;
    float f;
    struct {
        int x;
        int y[2];
    } point;
};

/* Multi-dimensional array with parenthesized size expression */
double complex_grid[(10+5)][20];

/* Function pointer with array parameter */
void (*sort_func)(int arr[], int size);

/* Typedef combining all three delimiters */
typedef struct {
    int (*methods[3])(void);
    union {
        int i;
        char c;
    } value;
} Object;

/* Additional cases to ensure full coverage */

/* Empty braces */
struct Empty {};

/* Array with empty initializer */
int empty_arr[5] = {};

/* Function pointer with no parameters */
void (*no_param_func)(void);

/* Nested parentheses in function pointer */
int (*(*(*nested_fp)(int))(double))(char);

/* Array of pointers to functions returning pointers to arrays */
int (*(*func_array[2])(void))[5];

/* Struct containing function pointer array */
struct HasFuncArray {
    int (*ops[4])(int, int);
};

/* Initializer with nested braces */
struct NestedInit {
    int a;
    struct {
        int b;
        int c;
    } inner;
} nested = {1, {2, 3}};
