/* Test input for gengtype parser coverage */
/* This file contains constructs to trigger consume_balanced() calls */

/* Parentheses case: Function pointer declarations */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_func_ptr)(void))(int);
int (*signal_handler)(int sig, void *ctx);

/* Brackets case: Array declarations */
int simple_array[10];
extern int multi_dim[5][(sizeof(int)*2)];
static char buffer[256];

/* Braces case: Aggregate definitions */
struct SimpleStruct {
    int field1;
    char field2;
};

union DataUnion {
    int i;
    float f;
    double d;
};

enum Color {
    RED,
    GREEN,
    BLUE
};

/* Nested/Combined cases */

/* Array of function pointers (combines [] and ()) */
int (*callbacks[5])(const char *);

/* Function pointer returning pointer to array (combines (), *, []) */
int (*(*get_matrix_ptr(void))[10]);

/* Struct with array member initialized in-line (combines {} and []) */
struct DataContainer {
    int values[3];
    char *names[2];
} global_data = { 
    .values = {1, 2, 3},
    .names = {"a", "b"}
};

/* Complex nested example */
typedef struct Node {
    struct Node *(*get_next)(void);
    int (*process)(int data[5]);
    union {
        int i;
        struct {
            int x, y;
        } point;
    } value;
} Node;

/* GTY-marked declarations (if gengtype recognizes GTY) */
typedef struct GTY(()) GcStruct {
    struct GcStruct *GTY((skip)) next;
    int data;
} GcStruct;

/* More complex function pointer with nested parentheses */
void (*(*(*ultimate_fp)(int (*)(char)))[5])(float);

/* Initializer with nested braces */
int matrix_init[2][3] = {{1, 2, 3}, {4, 5, 6}};

/* Typedef with all three delimiters */
typedef int (*(*array_func_ptr_t[3])(void))[5];
