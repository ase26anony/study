/* Test file for gengtype parser coverage - targeting parentheses, brackets, and braces */

/* 1. Parentheses cases - function pointers */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_func_ptr)(void))(int);
int (*signal(int sig, void (*handler)(int)))(int);

/* GTY-marked function pointer */
typedef void (*GTY(()) gty_callback)(void);

/* 2. Brackets cases - arrays */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
char *string_array[] = {"hello", "world"};

/* Array with GTY marker */
static GTY(()) int gty_array[100];

/* 3. Braces cases - struct/union/enum definitions */
struct SimpleStruct {
    int a;
    char b;
};

union SimpleUnion {
    int i;
    float f;
};

enum SimpleEnum { RED, GREEN, BLUE };

/* Static initializers with braces */
int global_init[3] = {1, 2, 3};
struct Point { int x; int y; } origin = {0, 0};

/* 4. Nested combinations - exercise consume_balanced recursion */

/* Array of function pointers (combines [] and ()) */
int (*callbacks[5])(const char*);

/* Function pointer returning pointer to array (combines (), *, and []) */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member initialized in-line (combines {} and []) */
struct Data { 
    int vals[2]; 
} data_instance = { .vals = {10, 20} };

/* Complex nested example */
struct Container {
    int (*processor)(int (*)(int), int);
    struct {
        char *items[5];
    } nested;
} container = {
    .processor = NULL,
    .nested = { .items = {"a", "b", "c"} }
};

/* 5. More GTY contexts with delimiters */
typedef struct GTY(()) TreeNode {
    struct TreeNode *GTY((skip)) left;
    struct TreeNode *right;
    int value;
} TreeNode;

/* Union with array and function pointer */
union GTY(()) ComplexUnion {
    int (*func_array[3])(void);
    struct {
        char data[256];
    } buffer;
};

/* 6. Edge cases with nested delimiters */
/* Multiple levels of parentheses */
int (*(*(*deep_func)(int))(char))(float);

/* Array with computed size containing parentheses */
int dynamic[(sizeof(int) + (alignof(double) - 1)) & ~(alignof(double) - 1)];

/* Struct containing array of structs containing arrays */
struct NestedArrays {
    struct {
        int matrix[2][2];
    } blocks[3];
};

/* 7. Function-like macro that might confuse parser (if not skipped properly) */
#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))
/* This should be skipped by the parser, but ensures we have parentheses */

/* 8. Enum with computed values in braces */
enum WithComputed {
    VAL1 = (1 << 0),
    VAL2 = (1 << 1),
    VAL3 = (1 << 2)
};

/* 9. Pointer to array */
int (*ptr_to_array)[10];

/* 10. Final complex example combining all three */
typedef struct GTY(()) MasterType {
    int (*methods[3])(struct MasterType *self, int arg);
    union {
        int (*int_func)(int);
        void (*void_func)(void);
    } callback;
    struct {
        char id[(32 + 1)];
        int scores[5];
    } metadata;
} MasterType;
