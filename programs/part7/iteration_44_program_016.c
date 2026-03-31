/* Test input for gengtype parser coverage - targeting balanced delimiter parsing */

/* 1. Parentheses () cases */
typedef int (*func_ptr)(int, char);
typedef void (*(*complex_fp)(void))(int);
typedef int (*array_of_funcs[5])(double);

/* GTY-marked function pointer */
static GTY(()) int (*global_callback)(const char *);

/* 2. Brackets [] cases */
extern int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
extern char *string_table[][3];

/* Array with function pointer elements */
static int (*callbacks[5])(const char*);

/* 3. Braces {} cases */
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

/* Initializers with braces */
static int global_vec[3] = {1, 2, 3};
static struct SimpleStruct s = { .a = 42, .b = 'X' };

/* 4. Nested and combined delimiter cases */

/* Array of function pointers (combines [] and ()) */
typedef int (*handler_array[10])(void *data);

/* Function pointer returning pointer to array (combines (), *, and []) */
typedef int (*(*get_matrix_ptr(void))[5][5]);

/* Struct with array member initialized in-line (combines {} and []) */
struct Container {
    int values[(2 + 3)];
    char *names[2];
};

static struct Container c = { 
    .values = {10, 20, 30, 40, 50},
    .names = {"first", "second"}
};

/* Complex nested example */
typedef struct Node {
    struct Node *next;
    void (*process)(struct Node *);
    int data[4];
} Node;

/* Even more complex: function pointer to function returning array pointer */
int (*(*(*ultimate_fp)(int x[2]))(void))[10];

/* Union with anonymous struct containing array */
union U {
    struct {
        int coords[3];
        char label[20];
    };
    double d;
};

/* Multi-dimensional array with parenthesized size expression */
extern int dynamic_array[][(sizeof(long) * 2)];

/* Function type with array parameter */
typedef int (*sort_func)(int arr[], int size);

/* Struct with bitfield and array */
struct Mixed {
    unsigned int flags : 4;
    int items[8];
    void (*cleanup)(struct Mixed *);
};

/* Final test: deeply nested combination */
static GTY(()) struct {
    int (*comparators[3])(const void *, const void *);
    union {
        struct {
            int matrix[2][2];
        };
        void *ptr;
    } data;
} global_config = {
    .comparators = { NULL, NULL, NULL },
    .data = { .matrix = {{1, 2}, {3, 4}} }
};
