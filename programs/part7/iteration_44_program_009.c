/* Test input for gengtype parser coverage */
/* This file contains constructs that trigger consume_balanced() calls */

/* GTY markers to ensure gengtype processes these declarations */
#define GTY(x) __attribute__((gty x))

/* 1. Parentheses cases - function pointers */
typedef int (*func_ptr_type)(int, char);
typedef void (*(*complex_func_ptr)(void))(int);
static GTY(()) int (*global_func_ptr)(double) = NULL;

/* 2. Brackets cases - arrays */
extern int simple_array[10];
static GTY(()) int multi_dim[5][(sizeof(int)*2)];
typedef int matrix_type[(10 + 5)][20];

/* 3. Braces cases - struct/enum definitions and initializers */
struct SimpleStruct {
    int field1;
    char field2;
};

enum TestEnum {
    VALUE1,
    VALUE2 = (1 << 3)
};

static GTY(()) struct SimpleStruct global_struct = { 42, 'A' };
static GTY(()) int initialized_array[3] = { 1, 2, 3 };

/* 4. Nested combinations */
/* Array of function pointers - combines [] and () */
int (*callback_array[5])(const char*);

/* Function pointer returning pointer to array - combines (), *, and [] */
int (*(*get_array_ptr_func(void))[10]);

/* Struct with nested array initializer - combines {} and [] */
struct DataContainer {
    int values[2];
    struct {
        float x;
        float y;
    } point;
};

static GTY(()) struct DataContainer data = { 
    .values = {10, 20}, 
    .point = {1.0, 2.0} 
};

/* 5. Complex nested example */
union ComplexUnion {
    int (*funcs[3])(int);
    struct {
        char *name;
        int (*handler)(void);
    } operations[2];
    long (*(*nested_func)(int))[5];
};

/* 6. More edge cases */
/* Parentheses in sizeof expressions within array bounds */
extern char buffer[sizeof(struct SimpleStruct) * 2];

/* Nested braces in struct initialization */
static GTY(()) struct {
    int a;
    struct {
        int b;
        int c;
    } inner;
} nested_struct = { 1, { 2, 3 } };

/* Function pointer with array parameter */
typedef int (*array_param_func)(int arr[], int size);

/* 7. Multiple levels of nesting */
typedef struct Node {
    struct Node *next;
    void (*methods[2])(struct Node*);
    union {
        int ival;
        float fval;
        struct {
            char *str;
            int len;
        } str_info;
    } data;
} NodeType;

/* 8. Declaration with all three delimiters in one */
static GTY(()) int (*(*all_in_one[3])(int))[5] = { NULL, NULL, NULL };

/* 9. Typedef with complex type involving all delimiters */
typedef struct {
    int (*compare)(const void *, const void *);
    void *items[10];
    struct {
        int count;
        int capacity;
    } metadata;
} ContainerType;

/* 10. Global variable with complex initializer */
static GTY(()) ContainerType global_container = {
    .compare = NULL,
    .items = { NULL },
    .metadata = { 0, 10 }
};
