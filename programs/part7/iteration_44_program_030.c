/* Test file to exercise gengtype's balanced delimiter parsing */

/* Parentheses cases - function pointers */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_fp)(void))(int);
int (*callback)(const char*, ...);

/* Brackets cases - arrays */
int array[10];
extern int matrix[5][(sizeof(int)*2)];
char buffer[BUFFER_SIZE];

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

/* Combined and nested cases */
int (*callbacks[5])(const char*);
int (*(*get_array_ptr(void))[10]);

struct Container {
    int (*func_ptr)(int);
    int values[3];
};

/* With GTY markers (if supported) */
typedef struct GTY(()) ListNode {
    struct ListNode *next;
    int data;
} ListNode;

/* Static initializers with braces */
int global_array[3] = {1, 2, 3};
struct Point points[2] = {{0, 0}, {1, 1}};

/* Complex nested example */
typedef struct GTY(()) Tree {
    struct Tree *left;
    struct Tree *right;
    int (*compare)(struct Tree *, struct Tree *);
    char name[(MAX_NAME_LEN+1)];
} Tree;

/* Function pointer array with initialization */
void (*handlers[])(void) = { NULL, NULL, NULL };

/* Multi-dimensional array in struct */
struct Matrix {
    int rows;
    int cols;
    double data[10][10];
};

/* Anonymous struct with array */
struct {
    int (*operations[3])(int, int);
    struct {
        int x[2];
        int y[2];
    } coords;
} anonymous_var = {
    .operations = { NULL, NULL, NULL },
    .coords = { .x = {0, 1}, .y = {2, 3} }
};
