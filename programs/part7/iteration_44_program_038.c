/* Test input for gengtype parser coverage */
/* This file contains constructs to trigger consume_balanced() calls */

/* 1. Parentheses cases - function pointers */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_fp)(void))(int);
int (*callback)(const char *, ...);

/* GTY-marked function pointer */
static GTY(()) int (*gty_func_ptr)(void);

/* 2. Brackets cases - arrays */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
char buffer[(1 << 8)];

/* Array with nested parentheses in size */
int sized_array[(int)(sizeof(double) + 1)];

/* 3. Braces cases - struct/union/enum definitions */
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

enum Color {
    RED,
    GREEN,
    BLUE
};

/* 4. Nested and combined delimiters */

/* Array of function pointers - combines [] and () */
int (*callbacks[5])(const char*);

/* Function pointer returning pointer to array - combines (), *, and [] */
int (*(*get_array_ptr(void))[10]);

/* Struct with array member initialized in-line - combines {} and [] */
struct Data {
    int vals[2];
} data_instance = { .vals = {10, 20} };

/* Complex nested example */
struct Container {
    int (*processor)(int (*)(int), int);
    struct {
        char *items[5];
    } nested;
} GTY((chain_next = "%h.next")) container_var;

/* 5. More GTY contexts with delimiters */
typedef struct GTY((tag("NODE"))) TreeNode {
    struct TreeNode * GTY((skip)) left;
    struct TreeNode *right;
    int values[3];
    void (*visit)(struct TreeNode *);
} TreeNode;

/* Union with GTY marker */
union GTY((desc("%1.type"))) Variant {
    int int_val;
    float float_val;
    char * GTY((length("%h.len"))) string_val;
};

/* 6. Pointer to array */
int (*ptr_to_array)[20];

/* 7. Function with array parameter (in typedef) */
typedef void (*sort_func)(int arr[], int size);

/* 8. Struct containing all delimiter types */
struct AllDelimiters {
    int (*func)(int);          /* () */
    int matrix[3][4];          /* [] */
    struct {                   /* {} */
        int x;
        int y;
    } point;
};

/* 9. Initializers with nested braces */
int init_array[3] = {1, 2, 3};
struct Point points[2] = {{1, 2}, {3, 4}};

/* 10. Complex declaration with multiple nesting */
void (*(*signal(int sig, void (*func)(int)))(int));

/* End of test input */
