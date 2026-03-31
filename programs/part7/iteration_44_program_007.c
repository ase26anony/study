/* Test file for gengtype parser coverage - targeting parentheses, brackets, and braces */

/* 1. Parentheses cases - function pointers */
typedef int (*simple_func_ptr)(int, char);
void (*(*complex_func_ptr)(void))(int);
int (*array_func_ptr[5])(double);

/* GTY-marked function pointer */
static GTY(()) int (*gty_hook)(const char *) = NULL;

/* 2. Brackets cases - arrays */
int simple_array[10];
extern int multi_dim[5][(sizeof(int)*2)];
char* string_array[] = {"test", "array"};

/* Array with function pointer elements */
int (*func_array[(10+2)])(void);

/* 3. Braces cases - struct/union/enum definitions */
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

enum Color { RED, GREEN, BLUE };

/* 4. Nested combinations */
/* Array of function pointers ([] and ()) */
int (*callbacks[5])(const char*);

/* Function pointer returning pointer to array ((), *, and []) */
int (*(*get_matrix_ptr(void))[10]);

/* Struct with array member initialized in-line ({} and []) */
struct Data { 
    int values[2]; 
    char* names[3];
} data_instance = { 
    .values = {10, 20}, 
    .names = {"a", "b", "c"}
};

/* Complex nested example */
typedef struct Node {
    struct Node *next;
    void (*callback)(int, struct Node*);
    int data[(sizeof(void*)*2)];
} Node;

/* Union with array and function pointer */
union ComplexUnion {
    int (*compute[3])(int, int);
    struct {
        int (*transform)(float);
        char buffer[256];
    } processor;
};

/* 5. More GTY contexts */
typedef GTY(()) struct GtyStruct {
    GTY(()) int *ptr_array[4];
    GTY(()) void (*methods[2])(void);
} GtyStruct;

/* External declaration with parentheses */
extern void (*(*external_api)(int param))[5];

/* Typedef combining all three delimiters */
typedef struct {
    int (*compare[2])(const void*, const void*);
    struct {
        int min;
        int max;
    } bounds;
} Container;
