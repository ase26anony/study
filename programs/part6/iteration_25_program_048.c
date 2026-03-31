/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn1)(void);
typedef fn1 (*fn2)(int);
int (*(*complex_func_ptr)(int (*)(double)))(char);

/* Helper function for function pointers */
int simple_func(void) { return 42; }
int (*func_returning_int(double d))(void) { return simple_func; }
int (*(*get_complex_func(void))(int (*)(double)))(char) {
    return complex_func_ptr;
}

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*matrix[3][4])(float, double);

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int (*callback)(int);
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    union {
        long l;
        double d;
    } u;
};

/* Global struct with complex initializer */
char global_char = 'A';
int callback_impl(int x) { return x * 2; }
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { &global_char, callback_impl },
    { .d = 3.14159 }
};

/* Array with designated initializers */
int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combine constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4} };

struct TableParam {
    int x;
    int y[2];
};

void process(int (*table[])[5], struct TableParam param);

/* 5. More global scope complex declarations */
int (*(*global_var)(void))[3];

struct Data {
    int (*func)(int);
    int (*methods[2])(struct Data*);
} global_data = { 
    callback_impl,
    { NULL, NULL }
};

/* Complex typedef chain */
typedef int (*Step1)(void);
typedef Step1 (*Step2)(int, int);
typedef Step2 (*Step3)(char*);
Step3 global_chain;

/* Function with deeply nested parameter */
void deep_nest(int (*(*(*arg1))(int))[5], 
               struct { 
                   union { 
                       int a; 
                       struct { 
                           char b[3]; 
                       } s; 
                   } u; 
               } arg2);

/* 6. Main function to use everything and prevent optimization */
int main(void) {
    int result = 0;
    
    /* Use global_nested */
    result += global_nested.a[0][0];
    result += global_nested.a[1][2];
    
    /* Use arr */
    result += arr[0][1];
    result += arr[1][0];
    
    /* Use ptr_to_array */
    result += ptr_to_array[0][0];
    result += ptr_to_array[1][1];
    
    /* Use global_data */
    if (global_data.func) {
        result += global_data.func(10);
    }
    
    /* Complex local declaration mirroring global patterns */
    int (*(*local_complex)(int (*)(double)))(char) = NULL;
    
    /* Array with parenthesized size */
    int (*local_array[(2*2)])[3];
    
    /* Nested struct initializer */
    struct Nested local_nested = {
        .a = {{7,8,9},{10,11,12}},
        .inner = { .p = &global_char, .callback = callback_impl },
        .u = { .l = 100 }
    };
    
    result += local_nested.a[0][2];
    result += local_nested.inner.callback(5);
    
    /* Compound literal with all delimiters */
    struct TableParam temp = {
        .x = 99,
        .y = { result, result + 1 }
    };
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}

/* Function definitions (even if not called, they're parsed) */
void process(int (*table[])[5], struct TableParam param) {
    /* Empty but parsed */
}

void deep_nest(int (*(*(*arg1))(int))[5], 
               struct { 
                   union { 
                       int a; 
                       struct { 
                           char b[3]; 
                       } s; 
                   } u; 
               } arg2) {
    /* Empty but parsed */
}
