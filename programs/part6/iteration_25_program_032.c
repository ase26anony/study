/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn1)(void);
typedef fn1 (*fn2)(int);
typedef int (*(*fn3)(int (*)(double)))(char);

/* Global function pointer using complex typedef */
fn2 global_fn2_ptr;

/* Even more complex: pointer to function returning pointer to array of function pointers */
int (*(*(*global_complex_func)(int, fn1))[5])(void);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*multi_dim_array[2][3])(int, int);

/* Array with parenthesized size expression */
double (*matrix[ (sizeof(int) > 2) ? 4 : 2 ])[(2+3)*2];

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int (*func_ptr)(int);
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
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { (char*)0x1000, NULL },
    { .d = 3.14159 }
};

/* Array with designated initializers */
int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combine constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

struct Anonymous {
    int x;
    int (*callback)(int (*)[3]);
};

void process(int (*table[])[5], struct Anonymous param);

/* Complex declaration mixing all delimiters */
int (*(*mixed_decl)(int))[2] = (int (*(*)(int))[2])0;

/* 5. Additional global scope complex declarations */
int (*(*global_var)(void))[3];

struct Data { 
    int (*func)(int); 
    struct {
        int (*nested_func)(int, int);
    } inner;
} global_data = { NULL, { NULL } };

/* Function compatible with our function pointer types */
int simple_func(int x) {
    return x * 2;
}

int func_returning_int(void) {
    return 42;
}

int (*func_taking_int_and_returning_int_ptr(int x))(int) {
    static int result;
    result = x;
    return &simple_func;
}

/* 6. Main function to use the complex types */
int main(void) {
    /* Use global_nested */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            sum += global_nested.a[i][j];
        }
    }
    
    /* Use arr */
    sum += arr[0][0] + arr[1][2];
    
    /* Use ptr_to_array */
    sum += ptr_to_array[0][0] + ptr_to_array[2][1];
    
    /* Assign to global function pointer */
    global_fn2_ptr = (fn2)func_taking_int_and_returning_int_ptr;
    
    /* Create local variable using complex typedef */
    fn3 local_fn3 = (fn3)0;
    
    /* Use global_data */
    global_data.func = simple_func;
    if (global_data.func) {
        sum += global_data.func(10);
    }
    
    /* Complex local declaration with initializer */
    int (*(*local_complex)(int))[3] = (int (*(*)(int))[3])0;
    
    /* Array of pointers to functions */
    int (*func_array[3])(int) = { simple_func, simple_func, simple_func };
    for (int i = 0; i < 3; i++) {
        if (func_array[i]) {
            sum += func_array[i](i);
        }
    }
    
    /* Nested struct with initializer */
    struct {
        int (*methods[2])(void);
        struct {
            int values[2][2];
        } inner;
    } local_struct = {
        { func_returning_int, func_returning_int },
        { {{1,2}, {3,4}} }
    };
    
    sum += local_struct.methods[0]();
    sum += local_struct.inner.values[1][1];
    
    /* Compound literal with nested braces */
    int (*another_ptr)[2][3] = &(int[2][3]){ { {1,2,3}, {4,5,6} } };
    sum += (*another_ptr)[0][1];
    
    printf("Result: %d\n", sum);
    return sum > 100 ? 0 : 1;
}

/* Function prototype with complex parameter (definition) */
void process(int (*table[])[5], struct Anonymous param) {
    if (table && param.callback) {
        /* Do nothing, just for syntax */
    }
}

/* Additional complex global with all delimiters */
struct Ultimate {
    int (*(*func_ptr_array[3])(int))[2];
    struct {
        int (*nested[2])(void);
    } inner;
    union {
        long (*long_func)(void);
        double (*double_func)(double);
    } u;
} global_ultimate = {
    { NULL, NULL, NULL },
    { { func_returning_int, func_returning_int } },
    { .long_func = NULL }
};
