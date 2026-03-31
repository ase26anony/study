/* Test file for gengtype parser coverage - targeting parentheses, brackets, and braces */

/* Case 1: Parentheses in function pointer declarations */
typedef int (*simple_func_ptr)(int, char);
typedef void (*(*complex_func_ptr)(void))(int);
typedef char *(*string_processor)(const char *input, int length);

/* Case 2: Brackets in array declarations */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
static char buffer[BUFFER_SIZE];

/* Case 3: Braces in aggregate definitions */
struct SimpleStruct {
    int field1;
    char field2;
};

union DataUnion {
    int int_val;
    float float_val;
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

/* Nested combinations for recursive consume_balanced calls */

/* Array of function pointers - combines [] and () */
int (*callbacks[5])(const char *);
void (*handlers[])(int, void *) = { NULL, NULL, NULL };

/* Function pointer returning pointer to array - combines (), *, and [] */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member - combines {} and [] */
struct Container {
    int values[3];
    char *(*processor)(int);
};

/* GTY-marked declarations (special gengtype context) */
typedef struct GTY(()) TreeNode {
    struct TreeNode *GTY((skip)) left;
    struct TreeNode *right;
    int data;
} TreeNode;

static GTY(()) int (*global_handler)(int) = NULL;
GTY(()) struct Container *global_container;

/* Complex nested example */
typedef struct GTY(()) ComplexType {
    int (*comparator)(const void *, const void *);
    void *data_array[20];
    union {
        struct {
            int x;
            int y[2];
        } point;
        float matrix[2][2];
    } variant;
} ComplexType;

/* Initializers with braces */
int initialized_array[3] = {1, 2, 3};
struct Container default_container = { 
    .values = {10, 20, 30},
    .processor = NULL 
};

/* Multi-dimensional array with parenthesized size expression */
extern double table[(10 + 5)][20];

/* Function pointer with array parameter */
void (*signal_handler)(int signals[], int count);

/* Typedef with all three delimiters */
typedef struct {
    int (*methods[4])(void);
    struct {
        char *name;
        int id;
    } info;
} Object;
