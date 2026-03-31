/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn_simple)(void);
typedef fn_simple (*fn_returning_fn)(int);
typedef int (*(*fn_complex)(int (*)(double)))(char);

/* Function to be pointed to by function pointers */
int simple_func(void) { return 42; }
int takes_double(double d) { return (int)d; }
int (*returns_fn_ptr(int (*f)(double)))(char) {
    static int result = 0;
    return (int (*)(char))&simple_func;
}

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*multi_array[2][3])(float);

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int values[2];
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    fn_simple func_ptr;
};

union ComplexUnion {
    struct Nested nested;
    int (*array_func[2])(void);
    long long big_value;
};

/* Global variables with initializers using nested braces */
struct Nested global_nested = { 
    {{1,2,3}, {4,5,6}}, 
    { "test", {7, 8} }, 
    &simple_func 
};

int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combine constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4} };

struct { 
    int x; 
    int (*func)(int); 
} param_struct = { 10, NULL };

void process(int (*table[])[5], struct { int x; } param) {
    /* Function body for completeness */
    (void)table;
    (void)param;
}

/* 5. Global scope complex declarations */
int (*(*global_func_ptr)(void))[3];
struct Data { 
    int (*func)(int); 
    int matrix[2][2];
} global_data = { NULL, {{1,2}, {3,4}} };

/* Complex typedef mixing all delimiters */
typedef struct {
    int (*(*member_func)(int))[4];
    union {
        int x;
        char (*str)[10];
    } u;
} UltimateType;

/* Another global with complex type */
UltimateType global_ultimate = {
    NULL,
    { .str = NULL }
};

/* Main function to use the types and prevent dead code elimination */
int main(void) {
    int result = 0;
    
    /* Use global_nested */
    result += global_nested.a[0][0];
    result += global_nested.a[1][2];
    result += global_nested.inner.values[0];
    
    /* Use arr */
    result += arr[0][1];
    result += arr[1][2];
    
    /* Use ptr_to_array */
    if (ptr_to_array) {
        result += 1;
    }
    
    /* Use global_data */
    result += global_data.matrix[0][0];
    result += global_data.matrix[1][1];
    
    /* Initialize and use function pointers */
    fn_simple local_fn_ptr = &simple_func;
    if (local_fn_ptr) {
        result += local_fn_ptr();
    }
    
    /* Initialize complex function pointer */
    fn_complex complex_ptr = &returns_fn_ptr;
    if (complex_ptr) {
        /* This would normally be called, but we just check it exists */
        result += 1;
    }
    
    /* Call process function */
    int (*local_table[2])[5] = { NULL, NULL };
    struct { int x; } local_param = { 5 };
    process(local_table, local_param);
    
    /* Print result to create observable side effect */
    printf("Result: %d\n", result);
    
    return 0;
}
