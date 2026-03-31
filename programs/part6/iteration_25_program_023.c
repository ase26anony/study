/* test_complex_types.c - Designed to exercise gengtype-parse.cc's 
   consume_balanced() function for all delimiter types */

/* 1. Complex function declarators with nested parentheses */
/* Pointer to function taking function pointer, returning pointer to function */
int (*(*complex_func_ptr)(int (*)(double)))(char);

/* Typedef chain building complex types */
typedef int (*fn1)(void);
typedef fn1 (*fn2)(int);
typedef fn2 (*fn3)(char, double);
fn3 global_fn_chain;

/* Function returning pointer to array of function pointers */
int (*(*func_ret_array(void))[5])(int);

/* 2. Multi-dimensional and complex array declarations */
/* Array of pointers to functions returning pointers to arrays */
int (*(*array_of_func_ptrs[5])(int))[10];

/* Array of pointers to arrays with parenthesized size expression */
char (*strings[(2+3)])[20];

/* Three-dimensional array with computed size */
int multi_dim_array[2 * (1+1)][3][(sizeof(int) > 4) ? 8 : 4];

/* 3. Nested aggregate initializers and type definitions */
/* Struct containing arrays and function pointers */
struct Nested {
    int a[2][3];
    struct {
        char *p;
        int (*func)(int, int);
    } inner;
    union {
        long l;
        double d;
    } data;
};

/* Global struct with complex initializer */
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { NULL, NULL },
    { .d = 3.14 }
};

/* Designated initializers with nested braces */
int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* Anonymous struct with nested array */
struct {
    struct {
        int x[2][2];
    } inner[2];
} anonymous_global = { .inner = { {{{1,2},{3,4}}}, {{{5,6},{7,8}}} } };

/* 4. Combine constructs in single declarations */
/* Variable with initializer using compound literal containing array */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

/* Function prototype with complex parameter */
void process(int (*table[])[5], struct { int x; int y[2]; } param);

/* Mixed declaration: pointer to array of structs containing function pointers */
struct Element {
    int (*compare)(const void *, const void *);
    void (*action)(void);
};

struct Element (*element_table[3])[2];

/* 5. Additional global declarations */
/* Global complex type */
int (*(*global_var)(void))[3];

/* Global struct with initializer */
struct Data { 
    int (*func)(int); 
    char name[20];
} global_data = { NULL, "test" };

/* 6. Helper functions for the complex types */
int simple_func(int x) { return x * 2; }
int func_for_ptr(int x) { return x + 1; }
int func_taking_double(double d) { return (int)d; }
int func_returning_int_char(char c) { return (int)c; }

/* Implementation of the process function */
void process(int (*table[])[5], struct { int x; int y[2]; } param) {
    /* Do nothing, just for declaration complexity */
    (void)table;
    (void)param;
}

/* 7. Main function to use the complex types and prevent dead code elimination */
int main(void) {
    int result = 0;
    
    /* Use global_nested struct */
    result += global_nested.a[0][0];  /* 1 */
    result += global_nested.a[1][2];  /* 6 */
    
    /* Use arr */
    result += arr[0][1];  /* 2 */
    result += arr[1][0];  /* 4 */
    
    /* Use anonymous_global */
    result += anonymous_global.inner[0].x[0][0];  /* 1 */
    result += anonymous_global.inner[1].x[1][1];  /* 8 */
    
    /* Use ptr_to_array */
    result += ptr_to_array[0][0];  /* 1 */
    result += ptr_to_array[2][1];  /* 6 */
    
    /* Initialize and use global_fn_chain */
    /* Create a compatible function chain */
    int local_func(void) { return 42; }
    fn1 f1 = local_func;
    fn2 f2 = (fn2)(void*)&f1;  /* Cast for demonstration */
    global_fn_chain = (fn3)(void*)&f2;
    
    /* Initialize array_of_func_ptrs */
    int (*(*local_func_ptr)(int))[10];
    /* We'll just take the address for demonstration */
    array_of_func_ptrs[0] = local_func_ptr;
    
    /* Initialize global_var */
    int (*local_array_creator(void))[3] {
        static int arr[3] = {10, 20, 30};
        return &arr;
    }
    global_var = local_array_creator;
    
    /* Call through global_var if initialized */
    if (global_var) {
        int (*arr_ptr)[3] = global_var();
        result += (*arr_ptr)[0];  /* 10 */
    }
    
    /* Initialize element_table */
    struct Element elem1 = { NULL, NULL };
    struct Element elem2 = { NULL, NULL };
    struct Element row[2] = { elem1, elem2 };
    element_table[0] = &row;
    
    /* Use global_data */
    global_data.func = simple_func;
    if (global_data.func) {
        result += global_data.func(5);  /* 10 */
    }
    
    /* Final result should be: 1+6+2+4+1+8+1+6+10 = 39 */
    printf("Result: %d\n", result);
    
    return 0;
}
