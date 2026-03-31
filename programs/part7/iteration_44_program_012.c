/* Test input for gengtype parser coverage */
/* This file contains constructs that trigger consume_balanced() calls */

/* GTY marker for gengtype recognition */
#define GTY(x) __attribute__((gty))

/* 1. Parentheses () cases */
typedef int (*func_ptr_type)(int, char);
void (*(*complex_func_ptr)(void))(int);
int (*signal_handler)(int signum);

/* 2. Brackets [] cases */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
char buffer[(16 + 8)];

/* 3. Braces {} cases */
struct SimpleStruct {
    int field1;
    char field2;
};

enum Color { RED, GREEN, BLUE };

/* 4. Nested and combined cases */
/* Array of function pointers - combines [] and () */
int (*callbacks[5])(const char*);

/* Function pointer returning pointer to array - combines (), *, [] */
int (*(*get_array_ptr_func(void))[10]);

/* Struct with array member - combines {} and [] */
struct DataContainer {
    int values[2];
    float matrix[3][3];
};

/* 5. With GTY markers (gengtype specifically looks for these) */
typedef GTY(()) struct Node {
    struct Node * GTY((skip)) next;
    int data;
} Node;

GTY(()) struct Tree {
    Node *root;
    int (* GTY((skip)) compare)(Node*, Node*);
};

/* 6. Complex nested example */
union ComplexUnion {
    int i;
    float f;
    struct {
        int x;
        int y;
    } point;
    void (*operation)(int, int);
};

/* 7. Initializers with braces */
static int initialized_array[3] = {1, 2, 3};
static struct DataContainer dc = { 
    .values = {10, 20}, 
    .matrix = {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}
};

/* 8. Function pointer in struct with array parameter */
struct Handler {
    int (*process)(int data[], int length);
    void (*cleanup)(void);
};

/* 9. Multi-dimensional array with parenthesized size */
int (*multi_array)[(4 * sizeof(int))];

/* 10. Typedef with all delimiters */
typedef struct {
    int (*methods[4])(void);
    union {
        int num;
        char str[16];
    } data;
} Object;
