/* Test input for gengtype parser coverage - targeting balanced delimiter parsing */

/* Case 1: Parentheses in function pointer declarations */
typedef int (*simple_func_ptr)(int, char);
typedef void (*(*complex_func_ptr)(void))(int);
typedef char *(*string_processor)(const char *input, int length);

/* Case 2: Brackets in array declarations */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
static double coords[3][4][5];

/* Case 3: Braces in aggregate definitions */
struct SimpleStruct {
    int field1;
    char field2;
};

union MixedUnion {
    int as_int;
    float as_float;
    struct {
        char byte1;
        char byte2;
    } as_bytes;
};

enum Color {
    RED,
    GREEN,
    BLUE
};

/* Nested combinations */

/* Array of function pointers ([] and ()) */
int (*callbacks[5])(const char *);
void (*handlers[])(int, void *) = { NULL, NULL };

/* Function pointer returning pointer to array ((), *, and []) */
int (*(*get_matrix_ptr(void))[10][20]);

/* Struct with array member initialized in-line ({} and []) */
struct Data {
    int values[2];
    char *names[3];
};

static struct Data dataset = { 
    .values = {10, 20},
    .names = {"a", "b", "c"}
};

/* Complex nested example */
typedef struct Node {
    struct Node *(*get_next)(void);
    void (*process)(int data[], int count);
    union {
        int (*int_func)(int);
        void (*void_func)(void);
    } operations;
} Node;

/* GTY-marked declarations (gengtype specifically looks for these) */
typedef struct GTY(()) GcStruct {
    struct GcStruct *GTY((skip)) next;
    int (*GTY((tag("FUNC_PTR"))) processor)(void);
    unsigned char data[100];
} GcStruct;

/* Multi-dimensional array with function pointer elements */
void (*(*signal_handlers[3][2])(int signum))(void);

/* Initializer with nested braces */
int initialized[2][3] = { {1, 2, 3}, {4, 5, 6} };

/* Typedef with all three delimiters */
typedef int (*(*complex_type[2])(int))[3];

/* Final test: everything combined */
struct MasterType {
    int (*comparator)(const void *, const void *);
    void *data_pool[100];
    struct {
        int count;
        char *items[];
    } flexible;
};
