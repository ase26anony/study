/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn_simple)(void);
typedef fn_simple (*fn_returning_fn)(int);
typedef int (*(*fn_complex)(int (*)(double)))(char);

/* Function to be pointed to by function pointers */
int simple_func(void) { return 42; }
int takes_double(double d) { return (int)d; }
int (*returns_fn_ptr(int (*f)(double)))(char) {
    static int result = 0;
    return (int (*)(char))&simple_func;
}

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*multi_array[2][3])(void);

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int values[2][2];
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    fn_simple func_ptr;
};

/* Global struct with nested initializer */
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { 
        "test", 
        {{7,8},{9,10}}
    },
    &simple_func
};

/* Union with complex type */
union DataUnion {
    int (*func_array[3])(void);
    struct {
        int x;
        int (*callback)(int);
    } s;
};

/* Array with designated initializers */
int designated_arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combine constructs in single declarations */
int (*combined_ptr)[2] = (int[][2]){ {1,2}, {3,4} };

struct Param {
    int x;
    int y[2];
};

void process(int (*table[])[5], struct Param param);

/* 5. Global scope complex declarations */
int (*(*global_func_ptr)(void))[3];

struct ComplexGlobal {
    int (*(*member_func)(int))[2];
    union DataUnion data;
} global_complex = { NULL, {{NULL, NULL, NULL}} };

/* Function pointer array with initializer */
int (*func_ptr_array[])(void) = { simple_func, simple_func, NULL };

/* 6. Main function to use all constructs */
int main(void) {
    /* Local variable using complex typedef */
    fn_returning_fn local_fn_ptr;
    
    /* Assign address to complex function pointer */
    fn_complex complex_ptr = &returns_fn_ptr;
    
    /* Access elements from nested initialized global */
    int sum = 0;
    sum += global_nested.a[0][0];      /* 1 */
    sum += global_nested.inner.values[1][1]; /* 10 */
    
    /* Use designated array */
    sum += designated_arr[0][2];       /* 3 */
    sum += designated_arr[1][0];       /* 4 */
    
    /* Use combined pointer */
    sum += combined_ptr[0][1];         /* 2 */
    sum += combined_ptr[1][0];         /* 3 */
    
    /* Call through function pointer */
    if (global_nested.func_ptr) {
        sum += global_nested.func_ptr(); /* 42 */
    }
    
    /* Initialize and use local function pointer */
    local_fn_ptr = (fn_returning_fn)&returns_fn_ptr;
    
    /* Use array of function pointers */
    if (func_ptr_array[0]) {
        sum += func_ptr_array[0]();    /* 42 */
    }
    
    /* Complex expression with all constructs */
    int result = sum + 
                 (global_nested.a[1][2] * 2) +  /* 6 * 2 = 12 */
                 (designated_arr[0][0] << 1);   /* 1 << 1 = 2 */
    
    printf("Result: %d\n", result);  /* Expected: 1+10+3+4+2+3+42+42+12+2 = 121 */
    
    /* Additional complex declarations inside function */
    struct {
        int (*nested_func[2])(struct { int x; });
        char (*strings[2])[10];
    } local_complex = { {NULL, NULL}, {NULL, NULL} };
    
    /* Compound literal with nested braces */
    int (*local_ptr)[3] = &(int[][3]){ {1,2,3}, {4,5,6} }[0];
    
    return 0;
}

/* Function definition for previously declared function */
void process(int (*table[])[5], struct Param param) {
    /* Empty implementation - just for declaration */
    (void)table;
    (void)param;
}

/* Additional global with all delimiter types combined */
struct UltimateType {
    int (*(*funcs[3])(int))[2];
    struct {
        char (*name)[20];
        int values[2][2];
    } data;
    union {
        int (*callback)(void);
        struct { int x; } s;
    } u;
} ultimate_var = {
    {NULL, NULL, NULL},
    {NULL, {{0}}},
    {.callback = NULL}
};
