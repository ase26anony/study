/* Test input for gengtype parser coverage - targeting parentheses, brackets, and braces */

/* 1. Parentheses cases - function pointers */
typedef int (*simple_func_ptr)(int, char);
typedef void (*(*complex_func_ptr)(void))(int);
typedef char *(*string_processor)(const char *input, int len);

/* With GTY marker */
typedef GTY(()) struct {
    int (*compare)(const void *, const void *);
} sorter;

/* 2. Brackets cases - arrays */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
static char *string_table[][3] = {{"a", "b", "c"}, {"x", "y", "z"}};

/* 3. Braces cases - struct/enum definitions and initializers */
struct Point {
    int x;
    int y;
    int z;
};

enum Color { RED = 1, GREEN = 2, BLUE = 3 };

union Data {
    int i;
    float f;
    struct {
        int x;
        int y;
    } point;
};

/* 4. Nested/combined cases */
/* Array of function pointers (combines [] and ()) */
int (*callbacks[5])(const char *);

/* Function pointer returning pointer to array (combines (), *, and []) */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member initialized in-line (combines {} and []) */
struct Container {
    int values[3];
    char *names[2];
} container = { 
    .values = {1, 2, 3}, 
    .names = {"first", "second"}
};

/* Complex nested example */
typedef struct Node {
    struct Node * GTY((skip)) children[4];
    void (* GTY((tag("NODE_TYPE"))) methods[2])(struct Node *);
    union {
        int int_val;
        struct {
            float x;
            float y;
        } coords;
    } data;
} Node;

/* Multi-level function pointer with arrays */
void (*(*signal_handlers[10])(int signum))[2];

/* Initializer with all three delimiters */
struct ComplexInit {
    int (*func)(int);
    int array[2][2];
} complex = {
    .func = NULL,
    .array = {{1, 2}, {3, 4}}
};

/* GTY-marked structure with nested types */
typedef GTY(()) struct Tree {
    struct Tree *left;
    struct Tree *right;
    char * GTY((length("strlen(%h.name)+1"))) name;
    int (*visitor)(struct Tree *);
} Tree;

/* Function prototype with array parameter */
extern int process_matrix(int matrix[][10], int (*callback)(int));

/* Typedef combining all three */
typedef struct {
    int (*methods[3])(void);
    struct {
        char *keys[10];
        int values[10];
    } map;
} Interface;
