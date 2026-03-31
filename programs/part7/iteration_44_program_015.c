/* Test input for gengtype parser coverage - targeting delimiter handling */

/* 1. Parentheses () - Function pointer declarations */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_func_ptr)(void))(int);
int (*signal(int sig, void (*handler)(int)))(int);

/* With GTY marker */
typedef GTY(()) int (*gty_func_ptr)(void);

/* 2. Brackets [] - Array declarations */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
char *string_array[] = {"hello", "world"};

/* Array with GTY marker */
static GTY(()) int gty_array[100];

/* 3. Braces {} - Aggregate definitions and initializers */
struct SimpleStruct {
    int a;
    char b;
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
int global_init[3] = {1, 2, 3};
struct SimpleStruct s = { .a = 42, .b = 'X' };

/* 4. Nested combinations - Exercise consume_balanced recursion */

/* Array of function pointers ([] and ()) */
int (*callbacks[5])(const char*);
void (*handlers[])(void) = { NULL, NULL };

/* Function pointer returning pointer to array ((), *, []) */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member initialized inline ({}, []) */
struct Container {
    int values[2];
    char *(*processor)(int);
} container = { 
    .values = {10, 20},
    .processor = NULL
};

/* Complex nested example */
typedef struct Node {
    struct Node *next;
    void (*action)(struct Node *);
    int data[(sizeof(void*) > 4) ? 8 : 4];
} Node;

/* GTY-marked complex type */
typedef GTY(()) struct Tree {
    struct Tree *left;
    struct Tree *right;
    int (*compare)(struct Tree *, struct Tree *);
    char name[32];
} Tree;

/* Even more complex nesting */
union UltraNested {
    struct {
        int (*(*func_array[3])(void))[5];
        struct {
            char data[100];
        } embedded;
    } s;
    long long int big;
};

/* Function prototype with complex parameter */
extern void register_callback(int (*cb)(int, char **), const char *name[]);

/* Multiple levels of parentheses */
int (*(*(*insane)(int (*)(double)))[10])(char);
