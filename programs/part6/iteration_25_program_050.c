/* test_complex_types.c
 * Complex C declarations to exercise gengtype-parse.cc's consume_balanced().
 * Target: lines 341-352 in gengtype-parse.cc
 */

#include <stdio.h>

/* ==================== 1. Complex Function Declarators with Nested Parentheses ==================== */

/* Base function type */
typedef int (*fn_simple)(void);

/* Function returning pointer to function */
typedef fn_simple (*fn_returns_fn_ptr)(int);

/* Pointer to function taking function pointer, returning pointer to function */
int (*(*complex_func_ptr)(int (*)(double)))(char);

/* Even more nested: pointer to function returning pointer to array of function pointers */
int (*(*(*nested_func_ptr)(void))[5])(int, int);

/* ==================== 2. Multi-Dimensional and Complex Array Declarations ==================== */

/* Array of pointers to functions returning pointers to arrays */
int (*(*array_of_func_ptrs[5])(int))[10];

/* Array of pointers to arrays, with parenthesized size expression */
char (*strings[(2+3)])[20];

/* Three-dimensional array with parenthesized subscript */
int matrix[3][(2+2)][5];

/* ==================== 3. Nested Aggregate Initializers and Type Definitions ==================== */

/* Struct containing arrays and function pointers */
struct Nested {
    int a[2][3];
    struct {
        char *p;
        double (*compute)(float);
    } inner;
    fn_returns_fn_ptr fn_field;
};

/* Union with nested struct and array */
union Container {
    struct {
        int (*callback)(void);
        short data[4];
    } s;
    long (*actions[2])(int);
};

/* Global struct with initializer using deeply nested braces */
struct Nested global_nested = {
    {{1,2,3}, {4,5,6}},
    { NULL, NULL },
    NULL
};

/* Array with designated initializers and nested braces */
int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* ==================== 4. Combine Constructs in Single Declarations ==================== */

/* Variable declaration with initializer using compound literal containing array */
int (*ptr)[2] = (int[][2]){ {1,2}, {3,4} };

/* Function prototype with complex parameter mixing all delimiters */
void process(int (*table[])[5], struct { int x; int (*handler)(int[]); } param);

/* Struct containing all three delimiter types in one member */
struct AllDelimiters {
    int (*func_array[3])(int[2]);
    struct { int (*ptr)(void); } nested;
};

/* ==================== 5. Additional Global Declarations ==================== */

/* Global complex type */
int (*(*global_var)(void))[3];

/* Global union with initializer */
union Container global_container = { .s = { NULL, {0,1,2,3} } };

/* Global array of struct pointers with nested initializer */
struct Nested *global_ptrs[2] = { &global_nested, NULL };

/* ==================== Helper Functions for Execution ==================== */

int simple_func(void) {
    return 42;
}

double compute_something(float f) {
    return (double)f * 2.0;
}

int func_for_array(double d) {
    return (int)d;
}

char func_returning_char(int x) {
    return (char)(x + 64);
}

/* ==================== main() - Uses Complex Types ==================== */
int main(void) {
    /* 1. Use complex typedef */
    fn_returns_fn_ptr local_fn_ptr = NULL;
    
    /* 2. Assign address to complex function pointer */
    complex_func_ptr = NULL; /* Would need matching function, but NULL exercises parsing */
    
    /* 3. Access elements from nested array/structure */
    int sum = 0;
    sum += global_nested.a[0][0];      /* Value: 1 */
    sum += arr[1][2];                  /* Value: 6 */
    sum += ((int*)ptr)[2];             /* Value: 3 (third element of compound literal) */
    
    /* 4. Use union member */
    sum += global_container.s.data[2]; /* Value: 2 */
    
    /* 5. Call through function pointer if non-NULL */
    if (simple_func) {
        sum += simple_func();          /* Value: 42 */
    }
    
    /* 6. Use array of function pointers (even if NULL) */
    if (array_of_func_ptrs[0]) {
        /* Not executed, but type is parsed */
    }
    
    /* 7. Complex local declaration mirroring global patterns */
    int (*(*local_complex)(int (*)(double)))(char) = NULL;
    struct AllDelimiters local_mixed = {
        { NULL, NULL, NULL },
        { NULL }
    };
    
    /* Final calculation and output */
    printf("Result: %d\n", sum);  /* Expected: 1 + 6 + 3 + 2 + 42 = 54 */
    return 0;
}

/* ==================== Unused but parsed declarations ==================== */

/* Unused function with complex parameter (parsed but not called) */
void process(int (*table[])[5], struct { int x; int (*handler)(int[]); } param) {
    /* Empty body - just for parsing */
}

/* Another global with deeply nested initializer */
struct AllDelimiters global_delimiters = {
    .func_array = { NULL, NULL, NULL },
    .nested = { .ptr = simple_func }
};

/* Array with nested braces in initializer */
int deep_array[2][2][2] = { { {1,2}, {3,4} }, { {5,6}, {7,8} } };
