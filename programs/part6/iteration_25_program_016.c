/* test_complex_types.c
 * This program is designed to exercise the balanced delimiter parsing
 * logic in gengtype-parse.cc, specifically targeting the consume_balanced
 * function calls for parentheses, brackets, and braces.
 */

#include <stdio.h>
#include <stdlib.h>

/* ==================== COMPLEX FUNCTION DECLARATORS ==================== */
/* Target: case '(': consume_balanced('(', ')') */

/* Level 1: Simple function pointer */
typedef int (*fn1)(void);

/* Level 2: Pointer to function returning a function pointer */
typedef fn1 (*fn2)(int);

/* Level 3: Function returning pointer to array of function pointers */
typedef int (*(*fn3)(double))[3];

/* Global complex function pointer declaration */
int (*(*global_func_ptr)(int (*)(double)))(char);

/* Helper function compatible with int (*)(double) */
int helper_func(double d) {
    return (int)(d * 2);
}

/* Function compatible with global_func_ptr's return type */
int (*func_returning_ptr_to_func(char c))(char) {
    static int (*ptr)(char) = NULL;
    return ptr;
}

/* ==================== COMPLEX ARRAY DECLARATIONS ==================== */
/* Target: case '[': consume_balanced('[', ']') */

/* Multi-dimensional array with parenthesized size expression */
char (*strings[(2+3)])[20];

/* Array of pointers to functions returning pointers to arrays */
int (*(*array_of_func_ptrs[5])(int))[10];

/* Complex array declaration with nested parentheses */
void (*signal(int sig, void (*func)(int)))(int);

/* ==================== NESTED AGGREGATE TYPES ==================== */
/* Target: case '{': consume_balanced('{', '}') */

/* Struct with nested arrays and function pointers */
struct Nested {
    int a[2][3];
    struct {
        char *p;
        void (*callback)(int, char);
    } inner;
    fn1 func_ptr;
};

/* Union with complex members */
union ComplexUnion {
    struct {
        int (*compute)(int, int);
        double data[2][2];
    } calc;
    struct Nested nested;
};

/* Global struct with initializer (triggers nested braces) */
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { NULL, NULL }, 
    NULL 
};

/* Array with designated initializers and nested braces */
int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* ==================== COMBINED CONSTRUCTS ==================== */
/* Mix all three delimiter types in single declarations */

/* Variable with compound literal initializer */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4} };

/* Function prototype with complex parameter */
void process(int (*table[])[5], struct { int x; int (*func)(int); } param);

/* Typedef combining all delimiters */
typedef struct {
    int (*(*member1)(void))[3];
    char (*member2[4])[10];
    union {
        struct { int a; } s;
        double d;
    } u;
} UltimateType;

/* Global instance with initializer */
UltimateType global_ultimate = {
    NULL,
    {NULL, NULL, NULL, NULL},
    { .s = {42} }
};

/* ==================== HELPER FUNCTIONS ==================== */

/* Simple function for function pointer assignment */
int simple_func(void) {
    return 42;
}

void sample_callback(int x, char c) {
    /* Do nothing for test purposes */
}

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    int result = 0;
    
    /* 1. Use complex typedef locally */
    fn2 local_fn2 = NULL;
    
    /* 2. Assign to global function pointer */
    global_func_ptr = func_returning_ptr_to_func;
    
    /* 3. Access nested array from global struct */
    result += global_nested.a[0][0];  /* 1 */
    result += global_nested.a[1][2];  /* 6 */
    
    /* 4. Access array with designated initializers */
    result += arr[0][2];  /* 3 */
    result += arr[1][0];  /* 4 */
    
    /* 5. Use compound literal through pointer */
    if (ptr_to_array) {
        result += ptr_to_array[0][1];  /* 2 */
        result += ptr_to_array[1][0];  /* 3 */
    }
    
    /* 6. Access union member from global complex type */
    result += global_ultimate.u.s.a;  /* 42 */
    
    /* 7. Assign and use function pointer */
    global_nested.func_ptr = simple_func;
    if (global_nested.func_ptr) {
        result += global_nested.func_ptr();  /* 42 */
    }
    
    /* 8. Initialize and use nested struct member */
    global_nested.inner.callback = sample_callback;
    global_nested.inner.p = "test";
    
    /* Total should be: 1+6+3+4+2+3+42+42 = 103 */
    printf("Result: %d\n", result);
    
    /* Verify expected result */
    if (result == 103) {
        printf("All complex type parsing exercised successfully.\n");
        return 0;
    } else {
        printf("Unexpected result: %d\n", result);
        return 1;
    }
}

/* Additional function definitions to satisfy references */
void process(int (*table[])[5], struct { int x; int (*func)(int); } param) {
    /* Implementation not critical for coverage */
    if (table && param.func) {
        param.func(param.x);
    }
}
