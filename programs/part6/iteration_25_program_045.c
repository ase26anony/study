/* test_complex_types.c
 * Complex C declarations to exercise gengtype-parse.cc's consume_balanced logic
 */

#include <stdio.h>

/* ==================== TARGET 1: NESTED PARENTHESES ==================== */
/* Function pointer types with multiple layers of parentheses */
typedef int (*fn1)(void);
typedef fn1 (*fn2)(int);
typedef int (*(*fn3)(double))(char);

/* Global complex function pointer declaration */
int (*(*global_func_ptr)(int (*)(double)))(char);

/* Compatible function for assignment */
int inner_func(double d) { return (int)d; }
int (*middle_func(int (*f)(double)))(char) {
    static int result = 65;
    (void)f;
    return (int (*)(char))&result; /* Simplified return */
}

/* ==================== TARGET 2: COMPLEX ARRAYS ==================== */
/* Multi-dimensional arrays with parenthesized sizes */
char (*string_array[(2+3)])[20];

/* Array of pointers to functions returning pointers to arrays */
int (*(*array_of_func_ptrs[5])(int))[10];

/* ==================== TARGET 3: NESTED STRUCTURES ==================== */
/* Struct with nested arrays and function pointers */
struct Nested {
    int a[2][3];
    struct {
        char *p;
        int (*func_ptr)(int, int);
    } inner;
    union {
        long l;
        double d;
    } data;
};

/* Global struct with deeply nested initializer */
struct Nested global_nested = {
    {{1,2,3}, {4,5,6}},
    { NULL, NULL },
    { .l = 42 }
};

/* Another struct with designated initializers */
struct ComplexArray {
    int arr[2][3];
    struct Nested nested;
};

struct ComplexArray global_complex = {
    .arr = { [0] = {1,2,3}, [1] = {4,5,6} },
    .nested = {
        .a = {{7,8,9},{10,11,12}},
        .inner = { .p = "test", .func_ptr = NULL },
        .data = { .d = 3.14 }
    }
};

/* ==================== TARGET 4: COMBINED CONSTRUCTS ==================== */
/* Declaration mixing all three delimiter types */
int (*combined_var)[2] = (int[][2]){ {1,2}, {3,4} };

/* Function prototype with complex parameter */
void process_table(int (*table[])[5], struct { int x; int y[2]; } param);

/* Union with anonymous struct containing array */
union Mixed {
    struct {
        int (*func)(int);
        char str[10];
    } s;
    void *ptr;
};

/* ==================== TARGET 5: ADDITIONAL COMPLEX TYPES ==================== */
/* Pointer to array of function pointers */
int (*(*ptr_to_func_array)[3])(void);

/* Function returning pointer to array of pointers to functions */
int (*(*(*get_func_matrix(void))[2])[3])(int);

/* Struct with flexible array member containing pointers */
struct WithFlexArray {
    int count;
    int *(*(*flex_array[])(void))[2];
};

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    int result = 0;
    
    /* 1. Use complex function pointer */
    global_func_ptr = middle_func;
    if (global_func_ptr != NULL) {
        result += 1;
    }
    
    /* 2. Access nested array elements */
    result += global_nested.a[0][0];      /* 1 */
    result += global_nested.a[1][2];      /* 6 */
    result += (int)global_nested.data.l;  /* 42 */
    
    /* 3. Access complex array structure */
    result += global_complex.arr[0][1];   /* 2 */
    result += global_complex.arr[1][2];   /* 6 */
    result += global_complex.nested.a[0][2]; /* 9 */
    
    /* 4. Use combined_var (pointer to array) */
    result += (*combined_var)[0];         /* 1 */
    result += (*(combined_var + 1))[1];   /* 4 */
    
    /* 5. Local variable with complex typedef */
    fn2 local_fn2 = NULL;
    fn3 local_fn3 = NULL;
    
    /* Prevent unused variable warnings */
    (void)local_fn2;
    (void)local_fn3;
    (void)string_array;
    (void)array_of_func_ptrs;
    (void)ptr_to_func_array;
    
    /* Final result: 1 + 1 + 6 + 42 + 2 + 6 + 9 + 1 + 4 = 72 */
    printf("Result: %d\n", result);
    
    return result == 72 ? 0 : 1;
}

/* Function definitions to satisfy references */
void process_table(int (*table[])[5], struct { int x; int y[2]; } param) {
    (void)table;
    (void)param;
}

int (*(*(*get_func_matrix(void))[2])[3])(int) {
    static int (*(*matrix[2])[3])(int) = { NULL, NULL };
    return matrix;
}
