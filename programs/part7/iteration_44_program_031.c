/* Test input for gengtype parser coverage - targeting balanced delimiter parsing */

/* 1. Parentheses () cases */
typedef int (*func_ptr)(int, char);
void (*(*complex_fp)(void))(int);
int (*signal(int sig, void (*handler)(int)))(int);

/* With GTY marker */
typedef GTY(()) struct {
    void (*callback)(void);
} CallbackHolder;

/* 2. Brackets [] cases */
int array[10];
extern int matrix[5][(sizeof(int)*2)];
char buffer[BUFSIZ];

/* Nested in struct */
struct ArrayContainer {
    int data[100];
    char *strings[20];
};

/* 3. Braces {} cases */
struct S { 
    int a; 
    char b; 
};

union U { 
    int i; 
    float f; 
    struct { 
        int x; 
    } s; 
};

int global_vec[3] = {1, 2, 3};
static const struct { int x; int y; } point = { .x = 10, .y = 20 };

/* 4. Combined and nested delimiter cases */

/* Array of function pointers - combines [] and () */
int (*callbacks[5])(const char*);

/* Function pointer returning pointer to array - combines (), *, [] */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member initialized in-line - combines {} and [] */
struct Data { 
    int vals[2]; 
    char *(*funcs[3])(void);
} d = { 
    .vals = {10, 20}, 
    .funcs = {NULL, NULL, NULL}
};

/* Complex nested example */
typedef struct TreeNode {
    struct TreeNode *children[(MAX_CHILDREN)];
    void (*visit)(struct TreeNode *);
    union {
        int ival;
        float fval;
        struct {
            char *str;
            int len;
        } sval;
    } data;
} TreeNode;

/* GTY-marked complex type */
typedef GTY(()) struct Graph {
    struct Graph *edges[];
    void (*traverse)(struct Graph *);
    struct {
        int weight;
        char label[50];
    } attributes;
} Graph;

/* Function pointer with array parameter */
void (*sorter)(int arr[], int size);

/* Multi-dimensional array with parenthesized size */
int (*grid)[(WIDTH * 2)];

/* Enum with last element (some gengtype contexts care about enums) */
enum Flags {
    FLAG_A = 1 << 0,
    FLAG_B = 1 << 1,
    FLAG_C = 1 << 2
};

/* Final complex declaration exercising all delimiters */
static GTY(()) struct {
    int (*comparators[3])(const void *, const void *);
    struct {
        char *name;
        int id;
    } entries[10];
} registry = {
    .comparators = {NULL, NULL, NULL},
    .entries = {{.name = "test", .id = 1}}
};
