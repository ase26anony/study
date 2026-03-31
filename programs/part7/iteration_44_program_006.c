/* Test input for gengtype parser coverage - targeting delimiter handling */

/* 1. Parentheses cases - function pointers */
typedef int (*simple_func_ptr)(int, char);
typedef void (*(*complex_fp)(void))(int);
typedef int (*array_of_funcs[5])(void);

/* GTY-marked function pointer */
static GTY(()) int (*global_handler)(const char *msg);

/* 2. Brackets cases - arrays */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
static char buffer[(1 << 8)];

/* Array with nested parentheses in size */
int sized_array[(10 + (5 * 2))];

/* 3. Braces cases - struct/union/enum definitions */
struct SimpleStruct {
    int field1;
    char field2;
};

union ComplexUnion {
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

/* 4. Nested/combined delimiter cases */

/* Array of function pointers - combines [] and () */
int (*callbacks[5])(const char*);

/* Function pointer returning pointer to array - combines (), *, [] */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member initialized inline - combines {} and [] */
struct Data { 
    int vals[2]; 
    char *names[(3 + 2)];
} data_instance = { 
    .vals = {10, 20}, 
    .names = {"a", "b", "c"}
};

/* Complex nested example */
typedef struct TreeNode {
    struct TreeNode *children[4];
    void (*visit)(struct TreeNode *);
    union {
        int int_val;
        float float_val;
    } data;
} TreeNode;

/* GTY-marked complex type */
GTY(()) struct GtyStruct {
    int (*methods[3])(void);
    struct {
        int x;
        int y[2];
    } coord;
} gty_instance = {
    .methods = {NULL, NULL, NULL},
    .coord = {0, {1, 2}}
};

/* Multi-dimensional array with complex size expression */
int complex_array[3][(sizeof(struct Data) / 2)][5];

/* Function pointer with array parameter */
typedef void (*sort_func)(int arr[], int size);

/* Anonymous struct in union */
union Container {
    struct {
        int (*callback)(void);
        int data[3];
    } s;
    long raw;
};

/* Final complex example hitting all delimiters */
typedef int (*(*ultimate_type[2])(int))[3];
