/* Complex type declarations to exercise gengtype parser's balanced delimiter handling */

/* 1. Complex function pointers with nested parentheses */
typedef int (*fn_simple)(void);
typedef fn_simple (*fn_returner)(int);
int (*(*complex_func_ptr)(int (*)(double)))(char);

/* Helper function for function pointers */
int simple_func(void) { return 42; }
int takes_double(double d) { return (int)d; }
char* returns_char_ptr(int x) { static char c = 'A' + x; return &c; }

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*matrix[3][4])(float);

/* 3. Nested structures with initializers */
struct Inner {
    char *p;
    int (*func)(int);
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
    { "test", &takes_double },
    { .d = 3.14 }
};

/* Array with designated initializers */
int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combined constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

struct Table {
    int rows;
    int cols;
};

void process(int (*table[])[5], struct Table param);

/* Global variable using complex type */
int (*(*global_var)(void))[3];

/* Another complex global with initializer */
struct Data { 
    int (*func)(int);
    int (*array[2])(void);
} global_data = { 
    &takes_double, 
    { &simple_func, &simple_func } 
};

/* 5. Even more complex typedef chain */
typedef int (*fn1)(int, int);
typedef fn1 (*fn2)(char*);
typedef fn2 (*fn3)[2];

/* Union with function pointer array */
union ComplexUnion {
    struct {
        int (*(*func_array[3])(void))(int);
        char (*string_ptrs[])[10];
    } s;
    long long values[4];
};

/* 6. Function with complex return type */
int (*(*get_processor(void))(int))[2] {
    static int arr[2] = {100, 200};
    static int (*ptr)[2] = &arr;
    static int (*(*func)(int))[2] = NULL;
    return func;
}

/* Main function to use the complex types */
int main(void) {
    /* Use global_nested */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            sum += global_nested.a[i][j];
        }
    }
    
    /* Use arr with designated initializers */
    sum += arr[0][0] + arr[1][2];
    
    /* Use ptr_to_array */
    sum += (*ptr_to_array)[0] + (*(ptr_to_array + 1))[1];
    
    /* Set up function pointers */
    fn_simple simple = &simple_func;
    sum += simple();
    
    /* Initialize global_var */
    static int local_array[3] = {7, 8, 9};
    int (*array_ptr)[3] = &local_array;
    /* Cannot directly assign to global_var as it needs a specific function type,
       but we can at least reference it to prevent dead code elimination */
    if (global_var) sum += 1;
    
    /* Use global_data */
    if (global_data.func) {
        sum += global_data.func(10);
    }
    if (global_data.array[0]) {
        sum += global_data.array[0]();
    }
    
    /* Complex local declaration */
    int (*(*local_complex)(int (*)(double)))(char) = NULL;
    if (local_complex) sum += 100;
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}

/* Function definitions */
void process(int (*table[])[5], struct Table param) {
    /* Dummy implementation */
    if (table && param.rows > 0) {
        (*table)[0][0] = param.cols;
    }
}
