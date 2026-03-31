/* Complex type declarations to exercise gengtype's balanced delimiter parsing */

/* 1. Complex function pointers with nested parentheses */
typedef int (*simple_fn)(void);
typedef simple_fn (*fn_returning_fn_ptr)(int);
typedef int (*(*complex_func_ptr)(int (*)(double)))(char);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*matrix[3][4])[5];

/* 3. Nested aggregate types with initializers */
struct Inner {
    char *p;
    int (*func)(int, int);
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    union {
        long x;
        double y;
    } u;
};

/* Global struct with nested initializer */
struct Nested global_nested = { 
    {{1, 2, 3}, {4, 5, 6}}, 
    { NULL, NULL }, 
    { .x = 42 } 
};

/* Array with nested designated initializers */
int arr[2][3] = { [0] = {1, 2, 3}, [1] = {4, 5, 6} };

/* 4. Combined constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1, 2}, {3, 4} };

struct Config {
    int (*table[3])[4];
    fn_returning_fn_ptr generator;
};

/* Function with complex parameter types */
void process(int (*table[])[5], struct Config *cfg) {
    /* Function body - will be implemented if needed */
}

/* 5. More global declarations mixing all delimiters */
int (*(*global_var)(void))[3] = NULL;

struct Data {
    int (*func)(int);
    int values[2][2];
} global_data = { 
    NULL, 
    {{10, 20}, {30, 40}} 
};

/* Complex typedef combining all delimiters */
typedef struct {
    int (*(*member1)(int))[2];
    struct Nested (*member2)[3];
} SuperComplexType;

/* 6. Helper functions for function pointers */
int add(int a, int b) {
    return a + b;
}

int process_double(double d) {
    return (int)(d * 2);
}

int process_char(char c) {
    return (int)c;
}

simple_fn get_simple_fn(void) {
    return NULL;
}

/* Main function to use the complex types */
int main(void) {
    /* Local variable using complex typedef */
    fn_returning_fn_ptr local_fn_ptr = NULL;
    
    /* Assign address to function pointer */
    struct Data local_data = global_data;
    
    /* Access elements from nested array */
    int sum = 0;
    sum += global_nested.a[0][0];  /* 1 */
    sum += global_nested.a[1][2];  /* 6 */
    sum += arr[0][1];              /* 2 */
    sum += arr[1][2];              /* 6 */
    
    /* Access struct with nested initializer */
    sum += global_data.values[0][0];  /* 10 */
    sum += global_data.values[1][1];  /* 40 */
    
    /* Use array pointer with compound literal */
    if (ptr_to_array) {
        sum += (*ptr_to_array)[0];  /* 1 if not optimized away */
    }
    
    /* Assign function pointers */
    local_data.func = &add;
    if (local_data.func) {
        sum += local_data.func(2, 3);  /* 5 */
    }
    
    /* Create and use complex function pointer chain */
    complex_func_ptr cfp = NULL;
    
    /* Print the result */
    printf("Result: %d\n", sum);  /* Expected: 1+6+2+6+10+40+5 = 70 */
    
    /* Additional complex local declarations */
    int (*(*local_complex)(int (*)(double)))(char) = NULL;
    char (*local_strings[5])[20] = {0};
    struct Nested local_nested = {
        .a = {{7, 8, 9}, {10, 11, 12}},
        .inner = { .p = "test", .func = &add },
        .u = { .y = 3.14 }
    };
    
    /* Use local nested struct */
    sum += local_nested.a[0][0];  /* 7 */
    
    printf("Final result: %d\n", sum);  /* Expected: 70 + 7 = 77 */
    
    return 0;
}
