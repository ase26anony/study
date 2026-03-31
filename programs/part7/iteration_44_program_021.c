/* Test file for gengtype parser coverage */
/* This file contains constructs to exercise the consume_balanced() function */

/* Parentheses () - Function pointer declarations */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_func_ptr)(void))(int);
int (*callback)(const char*, ...);

/* Brackets [] - Array declarations */
int simple_array[10];
extern int multi_dim[5][(sizeof(int)*2)];
char* string_array[] = {"hello", "world"};

/* Braces {} - Aggregate definitions */
struct SimpleStruct {
    int field1;
    char field2;
};

union TestUnion {
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

/* Nested and combined delimiters */
/* Array of function pointers - combines [] and () */
int (*func_array[5])(int);

/* Function pointer returning pointer to array - combines (), *, and [] */
int (*(*get_matrix(void))[10][20]);

/* Struct with array member - combines {} and [] */
struct DataContainer {
    int values[3];
    char* names[2];
};

/* GTY-marked declarations (if gengtype recognizes GTY) */
typedef struct GTY(()) ListNode {
    struct ListNode* GTY((skip)) next;
    int data;
} ListNode;

/* Complex nested example */
struct GTY(()) ComplexType {
    int (*compare)(struct ComplexType*, struct ComplexType*);
    void* items[10];
    union {
        int int_val;
        float float_val;
    } value;
};

/* Initializers with braces */
static struct DataContainer global_data = {
    .values = {1, 2, 3},
    .names = {"first", "second"}
};

/* Function pointer with array parameter */
void (*sort_func)(int arr[], int size);

/* Typedef with all three delimiters */
typedef struct {
    int (*methods[3])(void);
    struct {
        int x;
        int y[2];
    } coord;
} Object;

/* More edge cases */
/* Parentheses in sizeof expressions within array bounds */
double buffer[sizeof(long double)];

/* Nested braces in initializer */
struct NestedInit {
    struct {
        int a;
        int b;
    } inner;
} nested = { {1, 2} };

/* Pointer to array */
int (*ptr_to_array)[10];

/* Array of pointers to functions returning pointers to arrays */
int (*(*complex_array[2])(void))[5];
