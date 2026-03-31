/* Test input for gengtype parser coverage */
/* This file contains constructs that will trigger consume_balanced() calls */

/* Parentheses cases - function pointers */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_func_ptr)(void))(int);
int (*callback)(const char*, ...);

/* Brackets cases - arrays */
int simple_array[10];
extern int multi_dim[5][(sizeof(int)*2)];
char buffer[BUFFER_SIZE];

/* Braces cases - struct/enum definitions and initializers */
struct SimpleStruct {
    int field1;
    char field2;
};

enum Color { RED, GREEN, BLUE };

static int initialized_array[3] = {1, 2, 3};

/* Nested/Combined delimiter cases */
int (*array_of_funcs[5])(const char*);
int (*(*get_array_ptr(void))[10]);

struct Container {
    int (*member_func)(void);
    int data[4];
};

union Mixed {
    int i;
    float f;
    struct {
        int x;
        int y;
    } point;
};

/* GTY-marked declarations (gengtype specifically looks for these) */
typedef struct GTY(()) TreeNode {
    struct TreeNode *GTY((skip)) left;
    struct TreeNode *right;
    int value;
} TreeNode;

static GTY(()) int (*global_handler)(int) = NULL;

/* Complex nested example */
typedef int (*(*FactoryFunc)(int size))[(size > 0) ? size : 1];

/* Array with nested parentheses in size expression */
int computed_size[(sizeof(struct SimpleStruct) + 7) & ~7];

/* Function pointer with array parameter */
void (*sort_func)(int arr[], int size);

/* Struct with function pointer array */
struct Callbacks {
    int (*handlers[10])(void*);
    void (*cleanup)(void);
};

/* Initializer with nested braces */
struct Point {
    int x;
    int y;
} points[] = {{1, 2}, {3, 4}, {5, 6}};

/* Anonymous struct in union */
union Value {
    struct { int type; } header;
    struct { int type; int val; } integer;
};

/* Macro-like constructs (gengtype needs to skip these) */
#define MAX_SIZE (100)
typedef int ArrayType[MAX_SIZE];
