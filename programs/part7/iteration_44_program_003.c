/* Test input for gengtype parser coverage - targeting delimiter handling */

/* 1. Parentheses () cases */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_func_ptr)(void))(int);
int (*array_of_funcs[5])(const char*);

/* GTY-marked function pointer */
static GTY(()) int (*gty_hook)(int) = NULL;

/* 2. Brackets [] cases */
int simple_array[10];
extern int matrix[5][(sizeof(int)*2)];
struct ArrayHolder {
    int data[(10 + 2)];
    char *ptr_array[20];
};

/* 3. Braces {} cases */
struct SimpleStruct {
    int a;
    char b;
};

union ComplexUnion {
    int i;
    float f;
    struct {
        int x;
        char y[4];
    } nested;
};

enum TestEnum { VAL1, VAL2, VAL3 };

/* 4. Nested combinations */
/* Array of function pointers ([] and ()) */
int (*callbacks[5])(const char*);

/* Function pointer returning pointer to array ((), *, and []) */
int (*(*get_array_ptr(void))[10]);

/* Struct with initialized array member ({} and []) */
struct Data {
    int vals[2];
    char *names[3];
} data_instance = { 
    .vals = {10, 20},
    .names = {"a", "b", "c"}
};

/* 5. Complex nested example */
typedef struct TreeNode {
    struct TreeNode *children[(MAX_CHILDREN)];
    void (*operations[3])(struct TreeNode*);
    union {
        int int_val;
        float float_val;
        struct {
            char *str;
            int len;
        } string_data;
    } value;
} TreeNode;

/* 6. More GTY examples with various delimiters */
typedef GTY(()) struct GtyStruct {
    GTY(()) int *ptr_array[5];
    GTY(()) void (*callback)(struct GtyStruct*);
} GtyStruct;

/* 7. Multi-dimensional array with parenthesized size */
int complex_array[3][(2 + sizeof(void*))];

/* 8. Function pointer with array parameter */
void (*signal_handler)(int sig, const char *msg[(MAX_LINES)]);

/* 9. Anonymous struct in union with array */
union Container {
    struct {
        int ids[4];
        void (*handlers[2])(void);
    };
    char raw[32];
};

/* 10. Edge case: empty delimiters */
struct Empty { };
int empty_array[];
void (*null_func)(void);
