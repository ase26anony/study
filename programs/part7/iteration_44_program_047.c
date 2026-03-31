/* Test input for gengtype parser coverage - targeting parentheses, brackets, and braces */

/* GTY marker for gengtype recognition */
#define GTY(x) __attribute__((gty))

/* 1. PARENTHESES () cases */
/* Function pointer typedef */
typedef int (*func_ptr_type)(int, char);

/* Complex function pointer declaration */
void (*(*complex_fp)(void))(int);

/* GTY-marked function pointer */
static GTY(()) int (*gty_func_ptr)(double) = NULL;

/* 2. BRACKETS [] cases */
/* Simple array */
int simple_array[10];

/* Multi-dimensional array with expression */
extern int matrix[5][(sizeof(int)*2)];

/* Array in struct */
struct ArrayHolder {
    int data[(10 + 2)];
    char *names[20];
};

/* 3. BRACES {} cases */
/* Struct definition */
struct Point {
    int x;
    int y;
};

/* Union with nested struct */
union Value {
    int i;
    struct {
        float f;
        char c;
    } s;
};

/* Enum definition */
enum Color { RED, GREEN, BLUE };

/* Static initializer */
int global_init[3] = {1, 2, 3};

/* 4. NESTED/COMBINED cases */
/* Array of function pointers - combines [] and () */
int (*callbacks[5])(const char*);

/* Function pointer returning pointer to array - combines (), *, [] */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member initialized inline - combines {} and [] */
struct Data {
    int vals[2];
    char *(*processor)(int);
} data_instance = { 
    .vals = {10, 20}, 
    .processor = NULL 
};

/* Complex nested example */
struct Container {
    /* Array of pointers to functions taking array and returning struct */
    struct Point (*(*operations[3])(int[5]))(void);
    
    /* Union with array initializer */
    union {
        int nums[2];
        char *(*func)(void);
    } u = { .nums = {100, 200} };
};

/* GTY-marked complex type */
typedef GTY(()) struct Node {
    struct Node *next;
    void (*(*handler)(struct Node *))(int);
    int values[4];
} Node;

/* Even more complex: function returning pointer to array of function pointers */
int (*(*(*meta_func)(void))[5])(char *);

/* Initializer with all three delimiters */
struct Complete {
    int (*func)(int);
    int array[2];
    struct { int a; } nested;
} complete = { 
    .func = NULL, 
    .array = {0, 1}, 
    .nested = { .a = 42 } 
};
