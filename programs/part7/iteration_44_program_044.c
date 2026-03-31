/* Test file for gengtype parser coverage */
/* This file contains constructs that will trigger consume_balanced() calls */

/* Parentheses in function pointer declarations */
typedef int (*func_ptr)(int, char);
typedef void (*(*complex_fp)(void))(int);
typedef int (*callback)(const char*, ...);

/* Brackets in array declarations */
int array[10];
extern int matrix[5][(sizeof(int)*2)];
static char buffer[256];

/* Braces in aggregate definitions */
struct S { 
    int a; 
    char b; 
    int arr[3];
};

union U { 
    int i; 
    float f; 
    struct { 
        int x; 
    } s; 
};

enum E { 
    VALUE1, 
    VALUE2, 
    VALUE3 
};

/* Nested and combined delimiters */
int (*callbacks[5])(const char*);
int (*(*get_array_ptr(void))[10]);

/* Struct with GTY marker and nested delimiters */
struct GTY(()) Node {
    struct Node *next;
    int data;
    void (*handler)(struct Node*);
};

/* Global variables with various delimiters */
static GTY(()) int (*global_hook)(int) = NULL;
static GTY(()) struct S global_struct = { .a = 1, .b = 'x' };
static GTY(()) int global_vec[3] = {1, 2, 3};

/* Complex nested example */
typedef struct GTY(()) Container {
    int (*methods[3])(struct Container*, int);
    union {
        int i;
        struct {
            char *name;
            int id;
        } info;
    } data;
} Container;

/* Array of structs with initializer */
struct GTY(()) Point {
    int x;
    int y;
} points[2] = { {1, 2}, {3, 4} };

/* Function pointer returning pointer to array */
int (*(*factory(void))[5]);

/* Multi-dimensional array with size expression */
extern double grid[(10 + 5)][20];

/* Anonymous struct with bitfield */
struct {
    unsigned int flag:1;
    unsigned int value:7;
} GTY(()) flags = { .flag = 1, .value = 42 };
