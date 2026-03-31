/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn1)(void);
typedef fn1 (*fn2)(int);
typedef int (*(*fn3)(int (*)(double)))(char);

/* Global function pointer using fn3 type */
int (*(*global_func_ptr)(int (*)(double)))(char);

/* Helper function for function pointer assignment */
int helper_func(double d) { return (int)d; }
int (*helper_wrapper(int (*f)(double)))(char) {
    static int result = 0;
    return &result;
}

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*matrix[3][4])(void);

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int (*func)(int, int);
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    union {
        long l;
        double d;
    } value;
};

/* Global struct with complex initializer */
char global_char = 'A';
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { &global_char, NULL },
    { .d = 3.14159 }
};

/* Another struct with designated initializers */
struct ComplexArray {
    int arr[2][3];
    struct Nested *nested_ptr;
};

struct ComplexArray global_complex = { 
    .arr = { [0] = {1,2,3}, [1] = {4,5,6} },
    .nested_ptr = &global_nested
};

/* 4. Combine constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

/* Function with complex parameter types */
void process(int (*table[])[5], struct { int x; int y; } param) {
    /* Function body for completeness */
    (void)table;
    (void)param;
}

/* Another complex global */
struct Container {
    int (*func_table[3])(struct Nested*);
    void (*processor)(int (*)[5], struct { int x; int y; });
} global_container;

/* 5. Additional global scope complex declarations */
int (*(*global_var)(void))[3];

/* Union with nested struct and array */
union UltraComplex {
    struct {
        int (*(*func_arr[2])(int))[4];
        char (*str)[10];
    } s;
    long long data[4];
};

/* 6. Function using all the complex types */
int add_helper(int a, int b) { return a + b; }

int main(void) {
    /* 1. Use complex typedefs locally */
    fn1 local_fn1 = NULL;
    fn2 local_fn2 = NULL;
    fn3 local_fn3 = NULL;
    
    /* 2. Assign to global function pointer */
    global_func_ptr = helper_wrapper;
    
    /* 3. Access nested array elements */
    int sum = 0;
    sum += global_nested.a[0][0];  /* 1 */
    sum += global_nested.a[1][2];  /* 6 */
    sum += global_complex.arr[0][1];  /* 2 */
    sum += global_complex.arr[1][0];  /* 4 */
    
    /* 4. Use pointer to array */
    sum += ptr_to_array[0][0];  /* 1 */
    sum += ptr_to_array[2][1];  /* 6 */
    
    /* 5. Initialize and use function pointer in struct */
    global_nested.inner.func = add_helper;
    if (global_nested.inner.func) {
        sum += global_nested.inner.func(10, 20);  /* 30 */
    }
    
    /* 6. Complex local declaration with initializer */
    int (*(*local_complex)(int (*)(double)))(char) = global_func_ptr;
    
    /* 7. Array of pointers to functions */
    int (*func_array[3])(int, int) = { add_helper, add_helper, add_helper };
    sum += func_array[0](5, 5);  /* 10 */
    
    /* 8. Nested initializer in local variable */
    struct Nested local_nested = {
        .a = {{7,8,9},{10,11,12}},
        .inner = { .p = &global_char, .func = add_helper },
        .value = { .l = 100 }
    };
    sum += local_nested.a[0][0];  /* 7 */
    
    /* 9. Compound literal with nested braces */
    int (*dynamic_ptr)[3] = (int[][3]){ {13,14,15}, {16,17,18} };
    sum += dynamic_ptr[1][2];  /* 18 */
    
    /* Print the total sum (should be 1+6+2+4+1+6+30+10+7+18 = 85) */
    printf("Result: %d\n", sum);
    
    /* Additional complex expression to use more syntax */
    int result = (sum > 50) ? 
        ((int(*)(int, int))add_helper)(sum, 10) : 
        ((int(*)(int, int))add_helper)(sum, -10);
    
    printf("Final: %d\n", result);
    
    return 0;
}
