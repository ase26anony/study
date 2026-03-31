/* Complex type definitions to exercise gengtype parser's balanced delimiter handling */

/* 1. Complex function declarators with nested parentheses */
typedef int (*simple_func)(void);
typedef simple_func (*func_returning_func_ptr)(int);
typedef int (*(*complex_func_ptr)(int (*)(double)))(char);

/* Function to match complex_func_ptr type */
int (*func_for_double(double d))(char) {
    static int result = 0;
    return &result;
}

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*multi_dim_array[2][(3+2)])[4][5];

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int values[2][2];
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    simple_func func_ptr;
};

struct Nested global_nested = { 
    {{1,2,3}, {4,5,6}}, 
    { NULL, {{7,8}, {9,10}} }, 
    NULL 
};

/* Array with designated initializers */
int designated_arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combine constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4} };

/* Struct with function pointer array */
struct Combined {
    int (*func_array[3])(int);
    struct {
        char (*str_ptr)[10];
    } anonymous;
};

/* 5. Global scope complex declarations */
int (*(*global_var)(void))[3];

/* Function pointer with nested parameter */
void (*process_func)(int (*table[])[5], struct { int x; } param);

/* Union with complex members */
union ComplexUnion {
    int (*(*union_func_ptr)(void))[2];
    struct Nested nested_struct;
    char (*string_array[4])[15];
};

/* 6. Reference in main() to prevent dead code elimination */
int simple_callback(void) {
    return 42;
}

int another_callback(int x) {
    return x * 2;
}

int main(void) {
    /* Local variable using complex typedef */
    func_returning_func_ptr local_func_ptr = NULL;
    
    /* Assign address to complex function pointer */
    complex_func_ptr my_complex_func = func_for_double;
    
    /* Access elements from nested array/structure */
    int sum = 0;
    sum += global_nested.a[0][0];          /* 1 */
    sum += global_nested.inner.values[1][1]; /* 10 */
    sum += designated_arr[1][2];           /* 6 */
    
    /* Use the array pointer */
    sum += ptr_to_array[0][1];             /* 2 */
    
    /* Initialize and use function pointer array */
    struct Combined local_combined = { {NULL, NULL, NULL}, {NULL} };
    local_combined.func_array[0] = another_callback;
    if (local_combined.func_array[0]) {
        sum += local_combined.func_array[0](5); /* 10 */
    }
    
    /* Calculate final result */
    int result = sum + simple_callback(); /* 1+10+6+2+10+42 = 71 */
    
    /* Print result to create observable side-effect */
    printf("Result: %d\n", result);
    
    /* Additional complex local declaration */
    int (*(*local_var)(int))[3] = NULL;
    
    /* Compound literal with all delimiters */
    struct Nested temp = { 
        { {11,12,13}, {14,15,16} },
        { "test", {{17,18}, {19,20}} },
        simple_callback
    };
    
    return (result == 71) ? 0 : 1;
}

/* Additional global with initializer using all delimiters */
struct Combined global_combined = {
    { simple_callback, another_callback, NULL },
    { &((char[][10]){ "hello", "world" }[0]) }
};

/* Function prototype with deeply nested parameters */
void complex_prototype(
    int (*(*param1)(int (*)(double)))[3],
    struct Nested param2,
    char (*param3[][(2+3)])[10]
);

/* Typedef combining all delimiter types */
typedef struct {
    int (*(*func_table[2])(int))[3];
    union ComplexUnion data;
    char (*strings[{(2>1)?2:3}])[20];
} UltimateType;
