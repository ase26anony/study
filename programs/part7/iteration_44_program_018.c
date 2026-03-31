/* Test input for gengtype parser coverage */
/* This file contains constructs that trigger consume_balanced() calls */

/* GTY marker for gengtype recognition */
#define GTY(x) __attribute__((gty))

/* 1. Parentheses cases - function pointers */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_func_ptr)(void))(int);
int (*callback)(const char *);

/* GTY-marked function pointer */
static GTY(()) int (*gty_func_ptr)(void) = NULL;

/* 2. Brackets cases - arrays */
int simple_array[10];
extern int multi_dim[5][(sizeof(int)*2)];
char *string_array[] = {"test1", "test2"};

/* Array with GTY marker */
GTY(()) int gty_array[100];

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

enum Color {
    RED,
    GREEN,
    BLUE
};

/* Struct with GTY marker */
struct GTY(()) GtyStruct {
    struct GtyStruct *next;
    int value;
};

/* 4. Nested and combined cases */

/* Array of function pointers - combines [] and () */
int (*callbacks[5])(const char*);

/* Function pointer returning pointer to array - combines (), *, [] */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member initialized in-line - combines {} and [] */
struct Data {
    int vals[2];
    char *names[];
} data_instance = { 
    .vals = {10, 20},
    .names = {"a", "b", NULL}
};

/* Complex nested example */
typedef struct Node {
    struct Node * GTY((skip)) children[4];
    int (* GTY((tag("NODE_TYPE"))) methods[2])(void);
    union {
        int ival;
        float fval;
        struct {
            char *str;
            int len;
        } sval;
    } data;
} Node;

/* Even more complex - deeply nested */
void (*(*signal(int sig, void (*func)(int)))(int));

/* Multi-level array with function pointers */
int (*(*matrix[3][4])(float))[5];

/* Initializer with all three delimiters */
struct Complete {
    int (*func)(int);
    int array[3];
    struct {
        int x;
    } nested;
} complete = {
    .func = NULL,
    .array = {1, 2, 3},
    .nested = { .x = 42 }
};

/* Template for gengtype processing */
typedef struct GTY(()) Template {
    /* Array dimension in parentheses */
    int size[ (sizeof(void*) == 8) ? 2 : 1 ];
    
    /* Function pointer member */
    void (*cleanup)(struct Template *);
    
    /* Nested struct with initializer-like design */
    struct {
        int count;
        char *items[];
    } header;
} Template;

/* Final test - all delimiters in one declaration */
int (*(*all_in_one[2])(struct {int a; int b;}))[3] = {
    NULL,
    NULL
};
