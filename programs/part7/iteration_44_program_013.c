/* Test input for gengtype parser coverage */
/* This file contains constructs that trigger consume_balanced() calls */

/* GTY marker for testing */
#define GTY(x) __attribute__((gty))

/* 1. Parentheses cases - function pointers */
typedef int (*func_ptr_type)(int, char);
typedef void (*(*complex_func_ptr)(void))(int);

/* GTY-marked function pointer */
GTY(()) int (*global_callback)(const char *);

/* 2. Brackets cases - arrays */
int simple_array[10];
extern int multi_dim[5][(sizeof(int)*2)];

/* Array with GTY marker */
static GTY(()) int gty_array[20];

/* 3. Braces cases - struct/union/enum definitions */
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

enum Color { RED, GREEN, BLUE };

/* 4. Nested and combined cases */
/* Array of function pointers - combines [] and () */
int (*callbacks[5])(const char *);

/* Function pointer returning pointer to array - combines (), *, [] */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member and initializer - combines {} and [] */
struct Data {
    int values[2];
    char *names[(sizeof(int) + 1)];
};

/* Static initializer with braces */
static struct Data default_data = { 
    .values = {10, 20}, 
    .names = {"a", "b"} 
};

/* Complex nested case */
typedef struct Node {
    struct Node *next;
    void (*handler)(int, char);
    int scores[3];
} Node;

/* GTY-marked complex type */
GTY(()) Node *root = ((void*)0);

/* Even more complex - function pointer to function returning array pointer */
int (*(*(*ultimate_fp)(void))[5])(int);

/* Initializer with all delimiter types */
struct Container {
    int (*func)(int[2]);
    struct {
        char data[10];
    } inner;
} container = {
    .func = 0,
    .inner = { .data = "test" }
};
