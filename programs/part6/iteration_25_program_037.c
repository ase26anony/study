/* test_complex_types.c
 * Complex C declarations to exercise gengtype-parse.cc's consume_balanced()
 */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn1)(void);
typedef fn1 (*fn2)(int);
int (*(*complex_func_ptr)(int (*)(double)))(char);

/* Helper function for function pointers */
int simple_func(void) { return 42; }
int (*func_taking_double(double))(double) { return 0; }
int (*func_returning_ptr_to_func(char))(int) { return 0; }

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*matrix[3][4])(float, double);

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int (*callback)(int, int);
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    union {
        long l;
        double d;
    } data;
};

/* Global struct with initializer */
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { (char*)0x1000, 0 }, 
    { .l = 1000 } 
};

/* Array with designated initializers */
int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combine constructs in single declarations */
int (*ptr)[2] = (int[][2]){ {1,2}, {3,4} };

struct Anon {
    int x;
    int (*funcs[2])(void);
};

void process(int (*table[])[5], struct Anon param);

/* 5. Global scope declarations */
int (*(*global_var)(void))[3];
struct Data { 
    int (*func)(int); 
    struct {
        int a;
        int b[2][2];
    } nested;
} global_data = { 
    NULL, 
    { 10, {{1,2},{3,4}} } 
};

/* Complex typedef chain */
typedef int (*base_fn)(int);
typedef base_fn (*middle_fn)(char*);
typedef middle_fn (*outer_fn)(double);
outer_fn chain_var;

/* Even more complex: function returning pointer to array of function pointers */
int (*(*(*ultimate_func)(int))[5])(void);

/* 6. Main function to use these constructs */
int main(void) {
    /* Local variable using complex typedef */
    fn2 local_fn2 = 0;
    
    /* Assign to complex function pointer */
    complex_func_ptr = &func_returning_ptr_to_func;
    
    /* Access elements from nested global structure */
    int sum = global_nested.a[0][1] + global_nested.a[1][2];
    sum += global_data.nested.b[0][0] + global_data.nested.b[1][1];
    
    /* Use array with designated initializers */
    sum += arr[0][0] + arr[1][2];
    
    /* Use compound literal pointer */
    sum += ptr[0][0] + ptr[1][1];
    
    /* Assign to global_var (simulated) */
    int local_array[3] = {1, 2, 3};
    /* Cannot directly assign, but can use in expression */
    sum += local_array[0];
    
    /* Print result */
    printf("Result: %d\n", sum);
    
    /* Reference other globals to prevent dead code elimination */
    if (array_of_func_ptrs[0]) sum += 1;
    if (strings[0]) sum += 1;
    if (matrix[0][0]) sum += 1;
    
    return sum > 0 ? 0 : 1;
}

/* Additional function definitions for completeness */
void process(int (*table[])[5], struct Anon param) {
    /* Empty implementation - just for declaration */
    (void)table;
    (void)param;
}
