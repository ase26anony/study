/* Test file to exercise gengtype's balanced delimiter parsing */

/* 1. Parentheses () - Function pointer declarations */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_func_ptr)(void))(int);
int (*signal(int sig, void (*handler)(int)))(int);

/* GTY-marked function pointer */
static GTY(()) int (*gty_func_ptr)(const char *);

/* 2. Brackets [] - Array declarations */
int simple_array[10];
extern int multi_dim[5][(sizeof(int)*2)];
char *string_array[] = {"hello", "world"};

/* Array with function pointers */
int (*callback_array[5])(void);

/* 3. Braces {} - Aggregate definitions */
struct SimpleStruct {
    int field1;
    char field2;
};

union DataUnion {
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

/* 4. Nested combinations */

/* Array of function pointers ([] and ()) */
int (*func_ptr_array[3])(const char*);

/* Function returning pointer to array ((), *, and []) */
int (*(*get_matrix(void))[10][20]);

/* Struct with initialized array member ({} and []) */
struct Container {
    int values[4];
    struct {
        char *name;
        int id;
    } metadata;
} container = {
    .values = {1, 2, 3, 4},
    .metadata = {"test", 42}
};

/* Complex nested example */
typedef struct Node {
    struct Node *next;
    void (*callback)(struct Node*, int);
    union {
        int int_val;
        float float_val;
        struct {
            int x;
            int y;
        } coords;
    } data;
} Node;

/* GTY-marked complex type */
typedef GTY(()) struct Tree {
    struct Tree *left;
    struct Tree *right;
    int (*compare)(struct Tree*, struct Tree*);
    char key[32];
} Tree;

/* Function pointer with array parameter */
void (*sort_func)(int arr[], int size);

/* Multi-dimensional array initialization */
int matrix[2][3] = {
    {1, 2, 3},
    {4, 5, 6}
};

/* Anonymous struct in union */
union Value {
    struct {
        int type;
        union {
            int i;
            float f;
            char *s;
        } data;
    } tagged;
    void *ptr;
};

/* Pointer to array of function pointers */
int (*(*complex_array[2])[5])(void);
