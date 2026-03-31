/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn1)(void);
typedef fn1 (*fn2)(int);
typedef int (*(*fn3)(int (*)(double)))(char);

/* Global function pointer using fn3 type */
int (*(*global_func_ptr)(int (*)(double)))(char);

/* Helper function for function pointer assignment */
int helper_func(double d) { return (int)d; }
int (*func_returning_intptr(char c))(int) {
    static int (*result)(int) = 0;
    return result;
}

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*matrix[3][4])(float, double);

/* 3. Nested aggregate initializers and type definitions */
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
    } value;
};

/* Global struct with complex initializer */
char global_char = 'A';
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { &global_char, NULL },
    { .d = 3.14159 }
};

/* Another nested structure with array of function pointers */
struct Container {
    int (*(*func_array[2])(void))[3];
    struct {
        int x[2][2];
    } inner_struct;
};

/* 4. Combine constructs in single declarations */
/* Mixed delimiters in one declaration */
int (*complex_global)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

/* Function prototype with complex parameters */
void process(int (*table[])[5], struct { int x; int y[2]; } param);

/* Typedef combining all three delimiters */
typedef struct {
    int (*get_value)(int (*callback)(int, int), int param);
    char data[4][3];
} ComplexType;

/* 5. Additional global declarations */
/* Array of pointers to functions returning pointers to arrays */
int (*(*global_array[3])(void))[4];

/* Function returning pointer to array of function pointers */
int (*(*(*get_func_table(void))[5])(int, int));

/* Union with nested struct containing array */
union DataUnion {
    struct {
        int (*compare)(const char *, const char *);
        char buffer[10][20];
    } text;
    struct {
        double (*transform)(double);
        int matrix[2][2];
    } math;
};

/* 6. Main function to use these types and prevent dead code elimination */
int simple_func(int x, int y) { return x + y; }
double transform_func(double x) { return x * 2.0; }
int compare_func(const char *a, const char *b) { return 0; }

int main(void) {
    /* Use global function pointer */
    global_func_ptr = (int (*(*)(int (*)(double)))(char))0;
    
    /* Use array of function pointers */
    for (int i = 0; i < 5; i++) {
        array_of_func_ptrs[i] = NULL;
    }
    
    /* Access nested array from global struct */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            sum += global_nested.a[i][j];
        }
    }
    
    /* Use complex_global array */
    int (*local_ptr)[2] = complex_global;
    sum += local_ptr[0][0] + local_ptr[1][1];
    
    /* Create and use local variable with complex type */
    ComplexType local_complex;
    local_complex.get_value = NULL;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            local_complex.data[i][j] = (char)(i + j);
        }
    }
    
    /* Use union with nested types */
    union DataUnion data_union;
    data_union.text.compare = compare_func;
    data_union.math.transform = transform_func;
    
    /* Initialize and use matrix array */
    matrix[0][0] = NULL;
    matrix[1][2] = NULL;
    
    /* Initialize strings array */
    for (int i = 0; i < 5; i++) {
        strings[i] = NULL;
    }
    
    /* Use typedef chain */
    fn1 f1 = NULL;
    fn2 f2 = NULL;
    fn3 f3 = NULL;
    
    /* Create a Container struct with initializer */
    struct Container container = {
        .func_array = { NULL, NULL },
        .inner_struct = { .x = { {1,2}, {3,4} } }
    };
    
    /* Calculate final result using various accessed values */
    int result = sum 
               + (global_nested.inner.p != NULL ? 1 : 0)
               + (int)global_nested.value.d
               + container.inner_struct.x[0][0]
               + (int)local_complex.data[0][0];
    
    printf("Result: %d\n", result);
    return result > 0 ? 0 : 1;
}

/* Additional function definitions to satisfy references */
void process(int (*table[])[5], struct { int x; int y[2]; } param) {
    /* Empty implementation - just for declaration */
    (void)table;
    (void)param;
}

int (*(*(*get_func_table(void))[5])(int, int)) {
    static int (*(*table[5])(int, int))[4] = { NULL };
    return &table;
}
