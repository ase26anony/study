/* Complex type definitions to exercise gengtype parser's balanced delimiter handling */

/* 1. Complex function declarators with nested parentheses */
typedef int (*SimpleFunc)(void);
typedef SimpleFunc (*FuncReturningFunc)(int);
typedef int (*(*ComplexFuncPtr)(int (*)(double)))(char);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*multi_dim_array[2][3])(float);

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int (*func_ptr)(int, int);
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    union {
        long l;
        double d;
    } u;
};

/* Global struct with initializer using nested braces */
struct Nested global_nested = { 
    {{1,2,3}, {4,5,6}}, 
    { NULL, NULL }, 
    { .l = 42 } 
};

/* Array with designated initializers and nested braces */
int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combine constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4} };

struct Data {
    int (*func)(int);
    int matrix[2][2];
} global_data = { NULL, {{1,2},{3,4}} };

/* Function prototype with complex parameter */
void process(int (*table[])[5], struct { int x; int y[2]; } param);

/* 5. More global scope complex declarations */
int (*(*global_var)(void))[3];

/* Complex typedef chain */
typedef int (*FnA)(char);
typedef FnA (*FnB)(int, int);
typedef FnB (*FnC)(void);

/* 6. Helper functions for execution context */
int add(int x, int y) { return x + y; }
int process_double(double d) { return (int)(d * 2); }
int func_returning_int(char c) { return (int)c; }
int* func_returning_array_ptr(int x) { static int arr[3] = {1,2,3}; return arr; }
int (*func_returning_func_ptr(int x))(int) { return add; }

/* Main function to use the complex types and prevent dead code elimination */
int main(void) {
    /* Local variable using complex typedef */
    FuncReturningFunc local_func = NULL;
    
    /* Assign address to complex function pointer */
    ComplexFuncPtr complex_ptr = NULL;
    
    /* Access elements from nested array initialized globally */
    int sum = global_nested.a[0][0] + global_nested.a[1][2];  /* 1 + 6 = 7 */
    sum += arr[0][1] + arr[1][0];  /* 2 + 4 = 6, total 13 */
    
    /* Use the pointer to array with compound literal */
    sum += ptr_to_array[0][0] + ptr_to_array[1][1];  /* 1 + 4 = 5, total 18 */
    
    /* Access global struct */
    sum += global_data.matrix[0][0] + global_data.matrix[1][1];  /* 1 + 4 = 5, total 23 */
    
    /* Use union from nested struct */
    sum += (int)global_nested.u.l;  /* 42, total 65 */
    
    /* Create and use local complex type */
    int (*(*local_complex)(int (*)(double)))(char) = NULL;
    
    /* Array of pointers to functions */
    int (*func_array[3])(int) = { add, add, add };
    sum += func_array[0](10, 20);  /* 30, total 95 */
    
    /* Nested initializer in local scope */
    struct Nested local_nested = {
        {{7,8,9}, {10,11,12}},
        { (char*)&sum, add },
        { .d = 3.14 }
    };
    
    sum += local_nested.a[0][0];  /* 7, total 102 */
    
    /* Print result to create observable side-effect */
    printf("Result: %d\n", sum);
    
    /* Additional complex declarations in block scope to exercise parser further */
    {
        /* Mixed delimiters in single declaration */
        void (*signal(int sig, void (*func)(int)))(int);
        
        /* Complex array declaration with parenthesized size */
        int (*var_len_array[(sum % 5) + 2])[2];
        
        /* Nested struct with array of function pointers */
        struct {
            int (*callbacks[3])(void);
            struct {
                int data[2][2];
            } inner;
        } anon_struct = { {NULL, NULL, NULL}, {{{1,2},{3,4}}} };
    }
    
    return 0;
}

/* Additional global declarations for more coverage */
union OuterUnion {
    struct {
        int (*method)(struct Nested*);
        char data[10];
    } s;
    long long big_num;
};

/* Function with complex return type */
int (*(*get_complex_function(void))(int, int))(char) {
    return NULL;
}

/* Typedef with all three delimiters */
typedef struct {
    int (*operations[5])(int, int);
    union {
        char* str;
        void* ptr;
    } u;
    struct {
        int count;
        int (*handler)(void);
    } meta;
} ComplexType;

/* Global variable with the complex type and initializer */
ComplexType ct_instance = {
    { add, add, NULL, NULL, NULL },
    { .str = "test" },
    { 42, NULL }
};
