/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn_simple)(void);
typedef fn_simple (*fn_returning_fn)(int);
typedef int (*(*fn_complex)(int (*)(double)))(char);

/* Global function pointer using complex type */
int (*(*global_func_ptr)(int (*)(double)))(char);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*multi_dim_array[2][3])(void);

/* Array with parenthesized size expression */
double (*matrix[ (sizeof(int) > 4) ? 8 : 4 ])[16];

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int (*func_ptr)(int, int);
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    union {
        long x;
        double y;
    } data;
};

/* Global struct with deeply nested initializer */
struct Nested global_nested = { 
    {{1,2,3}, {4,5,6}}, 
    { (char*)0x1000, NULL },
    { .y = 3.14159 }
};

/* Array with designated initializers */
int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combine constructs in single declarations */
/* Compound literal with array */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

/* Struct with function pointer array */
struct Combined {
    int (*callbacks[3])(struct Combined*);
    struct {
        int count;
        char *names[2];
    } metadata;
};

/* Function prototype with complex parameters */
void process(int (*table[])[5], struct { int x; int y[2]; } param);

/* 5. Additional global declarations */
/* Pointer to array of function pointers */
int (*(*(*global_complex)[5])(void))[3];

/* Union with nested struct and array */
union UltraComplex {
    struct {
        int (*(*func_array[2])(int))[4];
        char data[2][3][4];
    } s;
    long long raw[8];
};

/* 6. Helper functions for function pointers */
int add(int a, int b) { return a + b; }
int simple_func(void) { return 42; }
int (*get_func_ptr(void))(void) { return simple_func; }

/* Array accessor function */
int* get_matrix_row(int idx) {
    static int row[5] = {1, 2, 3, 4, 5};
    return row;
}

/* Main function to use the complex types */
int main(void) {
    /* 1. Use complex function pointer type */
    fn_complex local_complex = NULL;
    
    /* 2. Access nested array elements */
    int sum = global_nested.a[0][0] + global_nested.a[1][2];  /* 1 + 6 = 7 */
    sum += arr[0][1] + arr[1][0];  /* 2 + 4 = 6, total = 13 */
    
    /* 3. Use array of pointers */
    char (*local_strings[2])[20];
    
    /* 4. Access compound literal */
    int val = ptr_to_array[1][0];  /* Should be 3 */
    sum += val;  /* total = 16 */
    
    /* 5. Use function pointers */
    fn_simple f1 = simple_func;
    fn_returning_fn f2 = NULL;
    
    /* 6. Complex array access */
    if (ptr_to_array) {
        sum += ptr_to_array[0][1];  /* Add 2, total = 18 */
    }
    
    /* 7. Nested struct access */
    sum += (int)global_nested.data.y;  /* Add 3, total = 21 */
    
    /* 8. Use multi-dimensional function pointer array */
    multi_dim_array[0][0] = simple_func;
    
    /* 9. Complex type in local declaration */
    int (*(*local_array[2])(int))[3];
    
    /* 10. Another complex local with initializer */
    struct {
        int (*funcs[2])(void);
        char data[2][2];
    } local_struct = { {simple_func, NULL}, {{'a','b'},{'c','d'}} };
    
    sum += local_struct.data[0][0] - 'a' + 1;  /* Add 1, total = 22 */
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", sum);
    
    /* Additional complex expressions to ensure parsing */
    int x = (int){5};
    int y = (int[2]){1,2}[0];
    
    /* Pointer to nested array */
    int (*nested_ptr)[2][3] = &global_nested.a;
    
    /* Function pointer assignment */
    global_nested.inner.func_ptr = add;
    if (global_nested.inner.func_ptr) {
        sum += global_nested.inner.func_ptr(2, 3);  /* Add 5, total = 27 */
    }
    
    printf("Final result: %d\n", sum);
    
    return sum > 20 ? 0 : 1;
}

/* Additional global with all delimiters combined */
struct Ultimate {
    int (*(*func)(int (*)[2]))[3];
    char data[ {2} ][3];  /* Note: using {} in array size is a GCC extension */
    struct {
        int x;
    } inner;
} ultimate_global = { NULL, {{'a'}}, {5} };

/* Function with complex return type */
int (*(*register_callback(int (*(*cb)(int))[2]))(void))[3] {
    static int arr[3] = {1, 2, 3};
    static int (*fixed_arr[2])[3] = { &arr, &arr };
    static int (*(*func_ptr)(void))[3] = NULL;
    
    /* This is intentionally complex to exercise the parser */
    return func_ptr;
}
