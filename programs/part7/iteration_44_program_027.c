/* Test input for gengtype parser coverage - targeting balanced delimiter parsing */

/* 1. Parentheses () cases - function pointers */
typedef int (*simple_func_ptr)(int, char);
typedef void (*(*complex_func_ptr)(void))(int);
typedef int (*array_func_ptr[5])(double);

/* GTY-marked function pointer */
static GTY(()) int (*gty_hook)(const char *) = NULL;

/* 2. Brackets [] cases - arrays */
int simple_array[10];
extern int multi_dim[5][(sizeof(int)*2)];
static GTY(()) char *string_table[] = {"a", "b", "c"};

/* 3. Braces {} cases - struct/union/enum definitions and initializers */
struct SimpleStruct {
    int x;
    char y;
};

union MixedUnion {
    int i;
    float f;
    struct {
        int tag;
    } nested;
};

enum Color { RED, GREEN, BLUE };

/* Static initializer with braces */
static GTY(()) int initialized_array[3] = {1, 2, 3};

/* 4. Nested combinations - exercises consume_balanced recursion */

/* Array of function pointers ([] containing ()) */
int (*callback_array[10])(void *arg);

/* Function pointer returning pointer to array (() containing [] containing ()) */
int (*(*get_matrix(void))[5][10])(float);

/* Struct with array member initialized inline ({} containing [] containing {}) */
struct DataContainer {
    int values[(2+3)];
    struct {
        char *name;
    } metadata;
} global_data = { 
    .values = {10, 20, 30},
    .metadata = { .name = "test" }
};

/* Complex nested type with all delimiters */
typedef struct Node {
    struct Node * GTY((skip)) next;
    int (* GTY((tag("NODE_TAG"))) methods[3])(struct Node *);
    union {
        int ival;
        float fval;
        char * GTY((length("strlen(%h.sval)+1"))) sval;
    } data;
} *NodePtr;

/* Multi-level pointer with array and function */
void (*(*signal_handlers[5])(int signum))[10];

/* Initializer with nested braces */
static struct {
    int a;
    struct {
        int b[2];
    } inner;
} nested_init = { .a = 1, .inner = { .b = {2, 3} } };

/* Typedef combining all three delimiters */
typedef int (*(*complex_type)[(sizeof(void*)/2)])(struct {int x;});
