/* test_complex_types.c - Complex declarations to exercise gengtype parser */

/* 1. Complex function pointer declarations with nested parentheses */
int simple_func(int x) { return x * 2; }

/* Function returning pointer to function taking function pointer */
int (*(*complex_func_ptr)(int (*)(double)))(char);

/* Typedef chain for complex function pointers */
typedef int (*fn1)(void);
typedef fn1 (*fn2)(int);
typedef fn2 (*fn3)(char*);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];

/* Array with parenthesized size expression */
char (*strings[(2+3)])[20];

/* 3D array with pointer elements */
int *matrix_3d[2][3][4];

/* 3. Nested structures with initializers */
struct Inner {
    char *p;
    int values[2][2];
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    double (*calc)(int, double);
};

/* Global struct with deeply nested initializer */
struct Nested global_nested = { 
    {{1,2,3}, {4,5,6}}, 
    { 
        "test", 
        {{10, 20}, {30, 40}}
    },
    NULL
};

/* Union with array and function pointer */
union ComplexUnion {
    int (*func_array[3])(void);
    struct {
        int x;
        int y[2][2];
    } data;
};

/* 4. Combined constructs in single declarations */
/* Compound literal with array */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

/* Function prototype with complex parameters */
void process(int (*table[])[5], struct { int x; int y[3]; } param);

/* Structure containing array of function pointers */
struct Container {
    int (*(*funcs[4])(void))[2];
    union ComplexUnion u;
};

/* 5. More global declarations */
/* Global variable with complex type */
int (*(*global_var)(void))[3];

/* Global struct with initializer */
struct Data { 
    int (*func)(int);
    int matrix[2][2];
} global_data = { 
    simple_func, 
    {{1,2}, {3,4}}
};

/* Typedef for extremely complex type */
typedef int (*(*(*complex_type)(int[][2]))(void))[5];

/* Array of structures with nested arrays */
struct Point {
    int x;
    int y;
    int z[2];
} points[3] = {
    {1, 2, {10, 20}},
    {3, 4, {30, 40}},
    {5, 6, {50, 60}}
};

/* 6. Function using designated initializers with nested braces */
int designated_array[2][3] = { 
    [0] = {1,2,3}, 
    [1] = {4,5,6} 
};

/* Function pointer array initialization */
int (*func_ptr_array[2])(int) = { simple_func, simple_func };

/* Main function to use the complex types */
int main(void) {
    int result = 0;
    
    /* 1. Use global_nested structure */
    result += global_nested.a[0][0];           /* 1 */
    result += global_nested.inner.values[1][1]; /* 40 */
    
    /* 2. Use designated_array */
    result += designated_array[0][2];          /* 3 */
    result += designated_array[1][0];          /* 4 */
    
    /* 3. Use global_data */
    if (global_data.func) {
        result += global_data.func(10);        /* 20 */
    }
    result += global_data.matrix[1][1];        /* 4 */
    
    /* 4. Use points array */
    result += points[0].z[1];                  /* 20 */
    result += points[2].x;                     /* 5 */
    
    /* 5. Use ptr_to_array */
    result += (*ptr_to_array)[1];              /* 2 */
    result += (*(ptr_to_array + 1))[0];        /* 3 */
    
    /* 6. Use func_ptr_array */
    if (func_ptr_array[0]) {
        result += func_ptr_array[0](5);        /* 10 */
    }
    
    /* Total should be: 1+40+3+4+20+4+20+5+2+3+10 = 112 */
    printf("Result: %d\n", result);
    
    /* Additional complex local declaration */
    int (*(*local_complex)(int))[3];
    
    /* Array with nested initializer */
    int nested_init[2][2][2] = { 
        { {1,2}, {3,4} }, 
        { {5,6}, {7,8} } 
    };
    result += nested_init[1][0][1];            /* 6 */
    
    printf("Final result: %d\n", result);      /* 118 */
    
    return 0;
}

/* Additional function definitions */
void process(int (*table[])[5], struct { int x; int y[3]; } param) {
    /* Function body not important for parser coverage */
    (void)table;
    (void)param;
}
