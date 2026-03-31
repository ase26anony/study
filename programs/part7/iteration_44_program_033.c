/* Test input for gengtype parser coverage */
/* This file contains constructs to exercise consume_balanced() */

/* Parentheses cases - function pointers */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_func_ptr)(void))(int);
int (*signal(int sig, void (*handler)(int)))(int);

/* Brackets cases - arrays */
int simple_array[10];
extern int multi_dim[5][(sizeof(int)*2)];
static char buffer[BUFSIZ];

/* Braces cases - aggregates and initializers */
struct SimpleStruct { 
    int a; 
    char b; 
};

union DataUnion {
    int i;
    float f;
    struct { int x; } nested;
};

enum Color { RED, GREEN, BLUE };

int initialized_array[3] = {1, 2, 3};
struct Point { int x; int y; } p = { .x = 10, .y = 20 };

/* Nested and combined cases */
int (*callbacks[5])(const char*);  /* Array of function pointers */
int (*(*get_matrix(void))[10]);    /* Function returning pointer to array */

struct Container {
    int (*compare)(const void*, const void*);
    void *data[100];
};

/* GTY-marked declarations (if gengtype recognizes them) */
typedef struct GTY(()) TreeNode {
    struct TreeNode *left;
    struct TreeNode *right;
    int value;
} TreeNode;

/* Complex nested example */
struct Complex {
    int (*operations[3])(int, int);
    struct {
        int (*getter)(void);
        void (*setter)(int);
    } methods;
    union {
        int arr[2][2];
        struct { int a; int b; } pair;
    } data;
};

/* Function pointer with array parameter */
void (*sort_func)(int array[], int size);

/* Pointer to array */
int (*ptr_to_array)[20];

/* Anonymous struct with bitfield */
struct {
    unsigned int flag:1;
    int values[4];
} anonymous_var = { .flag = 1, .values = {0, 1, 2, 3} };
