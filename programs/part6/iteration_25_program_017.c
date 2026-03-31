/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn_simple)(void);
typedef fn_simple (*fn_returning_fn)(int);
typedef int (*(*fn_complex)(int (*)(double)))(char);

/* Function to be pointed to */
int simple_func(void) { return 42; }
int (*func_taking_double(double x))(double) { 
    static int result = 0;
    return (int (*)(double))&result; 
}
int (*func_returning_int_ptr(char c))(void) {
    static int (*fp)(void) = simple_func;
    return fp;
}

/* Global variable using complex function pointer */
int (*(*global_func_ptr)(int (*)(double)))(char) = NULL;

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*matrix[3][(2+1)])(float, double);

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    float (*calc)(int, int);
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    union {
        long l;
        double d;
    } value;
};

/* Global struct with nested initializer */
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { NULL, NULL }, 
    { .d = 3.14159 } 
};

/* Array with designated initializers */
int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combine constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

struct TableParam {
    int x;
    double y;
};

void process(int (*table[])[5], struct TableParam param);

/* Complex declaration mixing all delimiters */
struct Container {
    int (*func_array[3])(struct { int a; int b; } param);
    char *(*get_name(void))[10];
} container_var = { {NULL, NULL, NULL}, NULL };

/* 5. Additional global declarations */
int (*(*global_var)(void))[3] = NULL;

struct Data { 
    int (*func)(int); 
    struct {
        char *name;
        int id;
    } info;
} global_data = { NULL, {"test", 100} };

/* Function to be called through function pointers */
int process_int(int x) { return x * 2; }
float calculate(int a, int b) { return (float)(a + b); }

/* Main function to use the complex types */
int main(void) {
    int result = 0;
    
    /* 1. Use complex function pointer */
    global_func_ptr = (int (*(*)(int (*)(double)))(char))func_returning_int_ptr;
    if (global_func_ptr) {
        int (*fp)(void) = global_func_ptr(func_taking_double);
        if (fp) {
            result += fp();  /* Should add 42 */
        }
    }
    
    /* 2. Access nested array elements */
    result += global_nested.a[0][0];  /* Add 1 */
    result += global_nested.a[1][2];  /* Add 6 */
    result += arr[0][1];              /* Add 2 */
    result += arr[1][0];              /* Add 4 */
    
    /* 3. Use array of pointers */
    if (ptr_to_array) {
        result += (*ptr_to_array)[0];  /* Add 1 */
        result += (*(ptr_to_array + 1))[1]; /* Add 4 */
    }
    
    /* 4. Access struct members */
    result += global_data.info.id;     /* Add 100 */
    
    /* 5. Assign and use function pointer in struct */
    global_nested.inner.calc = calculate;
    if (global_nested.inner.calc) {
        result += (int)global_nested.inner.calc(10, 5); /* Add 15 */
    }
    
    /* 6. Complex local declaration */
    int (*(*local_complex)(int (*)(double)))(char) = NULL;
    char (*(*local_array_ptr)[(3+2)])(int) = NULL;
    struct {
        int (*methods[2])(void);
        struct { int x; } nested;
    } local_struct = { {simple_func, NULL}, {7} };
    
    if (local_struct.methods[0]) {
        result += local_struct.methods[0]();  /* Add 42 */
    }
    result += local_struct.nested.x;          /* Add 7 */
    
    /* Final result: 42 + 1 + 6 + 2 + 4 + 1 + 4 + 100 + 15 + 42 + 7 = 224 */
    printf("Result: %d\n", result);
    
    return (result == 224) ? 0 : 1;
}

/* Function definition for previously declared function */
void process(int (*table[])[5], struct TableParam param) {
    /* Dummy implementation */
    if (table && param.x > 0) {
        (*table)[0][0] = param.x;
    }
}
