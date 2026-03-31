/* Test input for gengtype parser coverage - targeting delimiter handling */

/* Parentheses cases - function pointers */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_fp)(void))(int);
int (*callbacks[5])(const char*);

/* Brackets cases - arrays */
int array[10];
extern int matrix[5][(sizeof(int)*2)];
static char buffer[(1 << 8)];

/* Braces cases - aggregate definitions */
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

/* Combined and nested delimiters */
int (*(*get_array_ptr(void))[10]);

struct Container {
    int (*processor)(int);
    char name[32];
    struct {
        int values[3];
        float (*calc)(float, float);
    } nested;
};

/* With GTY markers (if supported in your gengtype version) */
typedef struct GTY(()) ListNode {
    struct ListNode *next;
    void *data;
} ListNode;

static GTY(()) int (*global_handler)(int) = NULL;

/* Complex nested example */
typedef struct GTY(()) Tree {
    struct Tree *children[4];
    int (*compare)(struct Tree *, struct Tree *);
    union {
        int int_val;
        float float_val;
        struct {
            char *name;
            int id;
        } named;
    } value;
} Tree;

/* Array of structs with initializer */
struct Point points[3] = {
    { .x = 1, .y = 2 },
    { .x = 3, .y = 4 },
    { .x = 5, .y = 6 }
};

/* Function pointer returning pointer to array */
int (*(*factory(void))[5])();

/* Multi-dimensional array with computed size */
int dynamic_matrix[][(2+3)] = {{1,2,3,4,5}, {6,7,8,9,10}};

/* Struct containing array of function pointers */
struct Operations {
    int (*math_ops[4])(int, int);
    void (*io_ops[2])(void);
};

/* Anonymous struct with nested arrays */
struct {
    int matrix[2][(sizeof(int))];
    struct {
        char *keys[10];
        int values[10];
    } map;
} anonymous_global = {
    .matrix = {{1,2}, {3,4}},
    .map = { .keys = {"a","b"}, .values = {1,2} }
};

/* Typedef with all three delimiters */
typedef int (*(*complex_type[2])(int))[3];

/* Final test - deeply nested */
struct DeepNest {
    int (*((*func_array[2])[3]))(void);
    struct {
        union {
            int (*callback)(struct DeepNest *);
            void *ptr;
        } u;
        char data[(10 * sizeof(int))];
    } inner;
};
