/* Test input for gengtype parser coverage */
/* This file contains constructs that will trigger consume_balanced() calls */

/* GTY marker for gengtype recognition */
#define GTY(x) __attribute__((gty x))

/* 1. Parentheses () cases - function pointers */
typedef int (*simple_func_ptr)(int, char);
typedef void (*(*complex_func_ptr)(void))(int);
static GTY(()) int (*global_func)(double) = NULL;

/* 2. Brackets [] cases - arrays */
extern int simple_array[10];
static GTY(()) int multi_dim[5][(sizeof(int)*2)];
typedef char string_array[][32];

/* 3. Braces {} cases - struct/enum definitions and initializers */
struct SimpleStruct {
    int field1;
    char field2;
};

enum TestEnum {
    VALUE1,
    VALUE2 = (1 << 3)
};

static GTY(()) struct SimpleStruct global_struct = { 42, 'A' };
static GTY(()) int init_array[3] = { 1, 2, 3 };

/* 4. Nested combinations */
/* Array of function pointers - combines [] and () */
int (*callback_array[5])(const char*);

/* Function pointer returning pointer to array - combines (), *, and [] */
int (*(*get_array_ptr(void))[10]);

/* Struct with nested array initializer - combines {} and [] */
struct DataContainer {
    int values[2];
    struct SimpleStruct nested;
};

static GTY(()) struct DataContainer container = { 
    .values = {10, 20}, 
    .nested = {100, 'B'} 
};

/* 5. Complex nested type with GTY marker */
typedef GTY(()) struct TreeNode {
    GTY(()) struct TreeNode *left;
    GTY(()) struct TreeNode *right;
    GTY(()) int (*compare)(struct TreeNode *, struct TreeNode *);
    GTY(()) int data[(sizeof(void*) * 2)];
} TreeNode;

/* 6. Union with nested struct */
union ComplexUnion {
    int as_int;
    float as_float;
    struct {
        int x;
        int y;
    } point;
    int (*operation)(union ComplexUnion *);
};

/* 7. Typedef with function pointer array */
typedef int (*(*SignalHandler[4])(int, void*))();

/* 8. Struct with array of structs */
struct NodeList {
    struct SimpleStruct items[8];
    int count;
};

/* 9. Declaration with all three delimiters in one */
static GTY(()) struct {
    int (*methods[3])(struct DataContainer *);
    union ComplexUnion data;
} global_object = {
    .methods = { NULL, NULL, NULL },
    .data = { .point = { 0, 0 } }
};

/* 10. Function-like macro with parentheses (should be skipped) */
#define MAX(a,b) ((a) > (b) ? (a) : (b))
