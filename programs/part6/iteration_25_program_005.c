/* Complex type declarations to exercise gengtype parser's balanced delimiter handling */

/* 1. Complex function pointers with nested parentheses */
typedef int (*simple_func)(void);
typedef simple_func (*func_returning_func_ptr)(int);
typedef int (*(*complex_func_ptr)(int (*)(double)))(char);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*multi_dim_array[2][3])[4][5];

/* 3. Nested structures with initializers */
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
    } u;
};

/* Global struct with nested initializer */
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { (char*)0x1000, NULL },
    { .d = 3.14159 }
};

/* Array with designated initializers */
int designated_arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combined constructs in single declarations */
int (*combined_ptr)[2] = (int[][2]){ {1,2}, {3,4} };

struct Config {
    int (*table[3])[5];
    func_returning_func_ptr processor;
};

/* Function with complex parameter */
void process(struct Config *cfg, int (*(*callback)(void))[3]);

/* 5. Global scope complex declarations */
int (*(*global_var)(void))[3];
struct Data { 
    int (*func)(int); 
    struct Nested nested;
} global_data = { NULL, {{0}} };

/* Simple compatible functions for function pointers */
int simple_func_impl(void) { return 42; }
int func_taking_int(int x) { return x * 2; }
int func_taking_double(double d) { return (int)(d * 10); }
int func_returning_int(char c) { return (int)c; }

/* Implementation of process function */
void process(struct Config *cfg, int (*(*callback)(void))[3]) {
    /* Use parameters to prevent optimization */
    if (cfg && callback) {
        /* Do nothing meaningful, just reference them */
        volatile int dummy = (int)(long)cfg->table[0];
        dummy += (int)(long)callback;
    }
}

int main(void) {
    /* 1. Use complex typedef */
    func_returning_func_ptr local_func_ptr = NULL;
    
    /* 2. Access nested array from global struct */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            sum += global_nested.a[i][j];
        }
    }
    
    /* 3. Use designated array */
    sum += designated_arr[0][1];
    
    /* 4. Use combined pointer */
    sum += combined_ptr[0][0] + combined_ptr[1][1];
    
    /* 5. Assign function pointers */
    global_data.func = func_taking_int;
    if (global_data.func) {
        sum += global_data.func(10);
    }
    
    /* 6. Create and use local complex declaration */
    int (*(*local_complex)(int (*)(double)))(char) = NULL;
    
    /* 7. Array of function pointers */
    int (*func_array[3])(int) = { func_taking_int, NULL, func_taking_int };
    for (int i = 0; i < 3; i++) {
        if (func_array[i]) {
            sum += func_array[i](i);
        }
    }
    
    /* 8. Nested structure with brace initializer */
    struct Inner local_inner = { 
        .p = "test",
        .func = func_taking_int
    };
    
    /* 9. Compound literal with nested braces */
    struct Config local_cfg = {
        .table = { NULL, NULL, NULL },
        .processor = NULL
    };
    
    /* 10. Reference all globals to prevent dead code elimination */
    volatile int prevent_opt = 0;
    if (array_of_func_ptrs[0]) prevent_opt++;
    if (strings[0]) prevent_opt++;
    if (multi_dim_array[0][0]) prevent_opt++;
    
    /* Print result to create observable side effect */
    printf("Result: %d\n", sum + prevent_opt);
    
    return 0;
}
