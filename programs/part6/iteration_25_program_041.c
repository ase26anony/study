/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn_simple)(void);
typedef fn_simple (*fn_returning_fn)(int);
typedef int (*(*fn_complex)(int (*)(double)))(char);

/* Function to be pointed to by function pointers */
int simple_func(void) { return 42; }
int func_taking_double(double d) { return (int)d; }
int (*func_returning_funcptr(int param))(char) {
    static int result = 100;
    return (int (*)(char))&result;
}

/* Global variable using complex function pointer type */
int (*(*global_func_ptr)(int (*)(double)))(char) = NULL;

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];

/* Array with parenthesized size expression */
int matrix[3 * (2 + 1)][4];

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int values[2][2];
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    fn_simple func_ptr;
};

/* Global struct with deeply nested initializer */
struct Nested global_nested = { 
    {{1, 2, 3}, {4, 5, 6}}, 
    { 
        "test", 
        {{7, 8}, {9, 10}}
    }, 
    &simple_func 
};

/* Union with nested struct and array */
union ComplexUnion {
    struct {
        int (*func_array[2])(void);
        double matrix[2][2];
    } s;
    long long raw[4];
};

/* Designated initializer with nested braces */
int arr[2][3] = { [0] = {1, 2, 3}, [1] = {4, 5, 6} };

/* 4. Combined constructs in single declarations */
/* Compound literal with array */
int (*ptr_to_array)[2] = (int[][2]){ {1, 2}, {3, 4} };

/* Function prototype with complex parameter */
void process_table(int (*table[])[5], struct { int x; int y[2]; } param);

/* Implementation of the complex function */
void process_table(int (*table[])[5], struct { int x; int y[2]; } param) {
    /* Function body - does nothing for test purposes */
    (void)table;
    (void)param;
}

/* 5. More global declarations mixing all delimiters */
/* Pointer to array of function pointers */
int (*(*complex_global)[3])(void);

/* Struct containing array of pointers to functions returning pointers to arrays */
struct UltimateType {
    int (*(*func_array[2])(int))[3];
    struct {
        union {
            int a;
            double b;
        } u;
    } nested;
};

/* 6. Main function to use the types and prevent dead code elimination */
int main(void) {
    int result = 0;
    
    /* Use global_nested */
    result += global_nested.a[0][0];  /* 1 */
    result += global_nested.a[1][2];  /* 6 */
    result += global_nested.inner.values[0][1];  /* 8 */
    
    /* Use arr with designated initializers */
    result += arr[0][2];  /* 3 */
    result += arr[1][0];  /* 4 */
    
    /* Use ptr_to_array (compound literal) */
    result += ptr_to_array[0][1];  /* 2 */
    result += ptr_to_array[1][0];  /* 3 */
    
    /* Initialize and use function pointers */
    global_func_ptr = &func_returning_funcptr;
    fn_simple local_fn = simple_func;
    result += local_fn();  /* 42 */
    
    /* Create and use a local struct with initializer */
    struct Nested local_nested = {
        {{11, 12, 13}, {14, 15, 16}},
        {"local", {{17, 18}, {19, 20}}},
        &simple_func
    };
    result += local_nested.a[0][2];  /* 13 */
    
    /* Use the array with parenthesized size */
    matrix[2][3] = 99;
    result += matrix[2][3];  /* 99 */
    
    /* Call process_table to use the function prototype */
    int (*table[2])[5];
    struct { int x; int y[2]; } param = { .x = 5, .y = {6, 7} };
    process_table(table, param);
    
    /* Final result should be: 1+6+8+3+4+2+3+42+13+99 = 181 */
    printf("Result: %d\n", result);
    
    return result == 181 ? 0 : 1;
}
