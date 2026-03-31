/* Test input for gengtype parser coverage */
/* This file contains constructs that will trigger consume_balanced() calls */

/* GTY marker for gengtype recognition */
#define GTY(x) __attribute__((gty))

/* 1. Parentheses cases - function pointers */
typedef int (*func_ptr_type)(int, char);
void (*(*complex_func_ptr)(void))(int);
int (*simple_fp)(double);

/* With GTY marker */
GTY(()) int (*gty_func_ptr)(const char*);

/* 2. Brackets cases - arrays */
int simple_array[10];
extern int multi_dim[5][(sizeof(int)*2)];
char* string_array[] = {"test", "array"};

/* Array with computed size */
int computed_size[(10 + 5) * 2];

/* 3. Braces cases - struct/union/enum definitions */
struct SimpleStruct {
    int field1;
    char field2;
};

union TestUnion {
    int i;
    float f;
    struct {
        int nested;
    } s;
};

enum Color { RED, GREEN, BLUE };

/* Static initializer with braces */
int initialized_array[3] = {1, 2, 3};
struct Point { int x; int y; } point = {10, 20};

/* 4. Nested combinations */
/* Array of function pointers - combines [] and () */
int (*callback_array[5])(const char*);

/* Function pointer returning pointer to array - combines (), *, [] */
int (*(*get_array_function(void))[10]);

/* Struct with nested array initializer - combines {} and [] */
struct Container {
    int values[2];
    struct {
        char* name;
        int id;
    } metadata;
} container = { 
    .values = {100, 200},
    .metadata = {"test", 42}
};

/* Complex nested example */
typedef struct Node {
    struct Node* (*get_next)(void);
    int data[(sizeof(void*) * 2)];
    union {
        struct {
            int x, y;
        } coord;
        float matrix[2][2];
    } u;
} Node;

/* GTY-marked struct with all delimiter types */
GTY(()) struct GtyStruct {
    void (*callback)(int);
    int array[5];
    struct {
        int count;
    } nested;
};

/* Multi-level pointer with parentheses */
int (*(*(**complex_ptr_arr[3])(int))[5])(void);

/* Initializer with all delimiter types */
struct AllDelimiters {
    int (*fp)(int);
    int arr[2];
    struct { int a; } s;
} all_delims = {
    .fp = 0,
    .arr = {1, 2},
    .s = {3}
};
