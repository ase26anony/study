/* test_gengtype_coverage.c
 * This file contains C declarations specifically designed to trigger
 * the balanced delimiter parsing logic in gengtype.
 */

/* 1. Parentheses () cases - function pointers */
typedef int (*simple_func_ptr)(int, char);
typedef void (*(*complex_fp)(void))(int);
typedef int (*array_of_funcs[5])(double);

/* GTY-marked function pointer */
static GTY(()) int (*global_callback)(const char*) = NULL;

/* 2. Brackets [] cases - arrays */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
static char buffer[(1 << 8)];

/* Array with nested parentheses in size expression */
int sized_array[(int)(sizeof(double) + 1)];

/* 3. Braces {} cases - struct/union/enum definitions */
struct SimpleStruct {
    int field1;
    char field2;
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
static int initialized_array[3] = {1, 2, 3};
static struct SimpleStruct s = { .field1 = 42, .field2 = 'A' };

/* 4. Nested and combined delimiter cases */

/* Array of function pointers - combines [] and () */
int (*callbacks[5])(const char*);

/* Function pointer returning pointer to array - combines (), *, and [] */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member and nested initializer - combines {} and [] */
struct Container {
    int values[2];
    struct SimpleStruct items[3];
};

static struct Container c = {
    .values = {10, 20},
    .items = {
        {1, 'a'},
        {2, 'b'},
        {3, 'c'}
    }
};

/* Complex nested type with all delimiters */
typedef struct Node {
    struct Node *next;
    void (*handler)(int, char*);
    int data[(sizeof(void*) * 2)];
} Node;

/* Even more complex: pointer to function returning pointer to array of structs */
struct ComplexType {
    struct Node (*(*get_node_array(int size))[10]);
};

/* GTY-marked struct with all delimiter types */
typedef GTY(()) struct GtyStruct {
    int (*methods[3])(void);
    struct Node *nodes;
    union DataUnion data;
} GtyStruct;

/* Multi-dimensional array with function pointer elements */
int (*(*signal_handlers[2][3])(int))[5];

/* Anonymous struct with nested arrays */
struct {
    int matrix[2][(2+1)];
    void (*cleanup)(void);
} anonymous_var = {
    .matrix = {{1,2,3},{4,5,6}},
    .cleanup = NULL
};

/* Enum with last comma (C99 feature) */
enum Flags {
    FLAG_A = 1 << 0,
    FLAG_B = 1 << 1,
    FLAG_C = 1 << 2,  /* trailing comma */
};
