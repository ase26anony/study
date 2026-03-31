/* Complex type definitions to exercise gengtype parser's balanced delimiter handling */

/* 1. Complex function declarators with nested parentheses */
typedef int (*simple_func)(void);
typedef simple_func (*func_returning_func_ptr)(int);
typedef int (*(*nested_func_ptr)(int (*)(double)))(char);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*multi_array[2][3])(float);

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int values[2][2];
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    simple_func func_ptr;
};

/* Global struct with initializer using nested braces */
struct Nested global_nested = { 
    {{1,2,3}, {4,5,6}}, 
    { NULL, {{7,8}, {9,10}} }, 
    NULL 
};

/* Array with designated initializers and nested braces */
int designated_arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combine constructs in single declarations */
int (*combined_ptr)[2] = (int[][2]){ {1,2}, {3,4} };

/* Union with complex members */
union ComplexUnion {
    int (*func_array[3])(void);
    struct {
        char (*string_ptrs[2])[10];
        int value;
    } data;
};

/* 5. Global scope complex declarations */
int (*(*global_var)(void))[3];
struct Data { 
    int (*func)(int); 
    union ComplexUnion u;
} global_data = { NULL, { .data = { NULL, 42 } } };

/* Function prototype with complex parameter */
void process(int (*table[])[5], struct { int x; int y[2]; } param);

/* Simple compatible functions for function pointers */
int simple_func_impl(void) { return 42; }
int func_taking_int(int x) { return x * 2; }
int func_taking_double(double d) { return (int)(d * 10); }
int (*func_returning_int_ptr(char c))(void) { 
    static int (*ptr)(void) = simple_func_impl;
    return ptr; 
}

/* 6. Main function to use the complex types and prevent dead code elimination */
int main(void) {
    /* Local variable using complex typedef */
    func_returning_func_ptr local_func_ptr = NULL;
    
    /* Assign address to complex function pointer */
    nested_func_ptr complex_ptr = NULL;
    
    /* Access elements from nested array/structure */
    int sum = 0;
    sum += global_nested.a[0][0];          /* Should be 1 */
    sum += designated_arr[1][2];           /* Should be 6 */
    sum += global_data.u.data.value;       /* Should be 42 */
    
    /* Use the combined_ptr */
    sum += combined_ptr[0][1];             /* Should be 2 */
    
    /* Create and use a local struct with initializer */
    struct Nested local_nested = {
        {{11,12,13}, {14,15,16}},
        { NULL, {{17,18}, {19,20}} },
        simple_func_impl
    };
    
    sum += local_nested.a[1][2];           /* Should be 16 */
    
    /* Call through function pointer if non-NULL */
    if (local_nested.func_ptr) {
        sum += local_nested.func_ptr();    /* Should be 42 */
    }
    
    /* Array of function pointers */
    int (*func_array[2])(float) = { NULL, NULL };
    
    /* Complex array declaration with initialization */
    int matrix[2][3][2] = { 
        { {1,2}, {3,4}, {5,6} },
        { {7,8}, {9,10}, {11,12} }
    };
    
    sum += matrix[1][2][1];                /* Should be 12 */
    
    /* Print the result */
    printf("Result: %d\n", sum);           /* Expected: 1+6+42+2+16+42+12 = 121 */
    
    return 0;
}

/* Implementation of process function */
void process(int (*table[])[5], struct { int x; int y[2]; } param) {
    /* Function body not important for coverage */
    (void)table;
    (void)param;
}
